#include "Session.h"
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <cassert>

using namespace boost;
using namespace boost::asio;

//����ȷ�����͵��ֽ�
char Byte[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

//�ǳ�����̬��Ա���������������ʼ��
//Server* Session::server = nullptr;

Session::Session(ios_type& ios) : socket(ios), isclose(false), dtimer(get_io_service()), gtimer(get_io_service()), server(Server::get_server()){}

void Session::Close() {
	boost::system::error_code ignored_ec;
	socket.shutdown(ip::tcp::socket::shutdown_both, ignored_ec);
	socket.close(ignored_ec);
	isclose = true;
	BOOST_LOG_TRIVIAL(info) << "�ر��������ID��" << u_id << "������";
	if(ignored_ec) {
		BOOST_LOG_TRIVIAL(error)<<ignored_ec.message()<<std::endl;
	}
}

void Session::ReadPlayerInfo() {
	async_read(socket, 
		read_buf.GetMsgBuf(bonus),            //������Ϸ�������Ϣ�ֽ���һ�£���������bonus��������������
		boost::bind(&Session::handle_read_player, shared_from_this(),
		placeholders::error, placeholders::bytes_transferred)
		);
}

void Session::handle_read_player (const boost::system::error_code& err, size_t byte_transferred) {
	if(err) {
		BOOST_LOG_TRIVIAL(error) << "����Ҷ�ȡ��Ϣʱ����" << err.message();
		Close();
		return;
	}
	//TODO Log
	read_buf.Retrive(byte_transferred);
	const char* data = read_buf.Peek();
	game_type = static_cast<Type>(data[0]);
	u_id = (((unsigned short)data[1]) << 8) + data[2];
	car_type = static_cast<Car>(data[3]);
	car_img = static_cast<CarImg>(data[4]);
	room_id = static_cast<int>(data[5]);
	read_buf.Consume(byte_transferred);
	if(room_id < 101 && room_id > 0) {
		--room_id;
		try {
			if(server.EnterRoom(game_type, room_id)) {//���뷿��ɹ�
				BOOST_LOG_TRIVIAL(info) << "���ID��" << u_id << "ͨ����������Ž���" << room_id + 1 << "�ŷ���";
				WriteRoomSuccess();//��ͻ��˷��ͷ�����Ϣ,�Լ�������Ϣ
				WriteHeartPackage();//��ʼ����������
			} else {
				BOOST_LOG_TRIVIAL(info) << "���ID��" << u_id << "��ͼ����" << room_id + 1 << "�ŷ���ʧ��";
				WriteRoomFail();//��ͻ��˷��ͽ��뷿��ʧ����Ϣ
			}
		} catch(std::exception& e) {
			BOOST_LOG_TRIVIAL(warning) << "���ID��" << u_id << "�����˴���ķ���Ż�����Ϸ���ͣ�" << e.what();
			WriteRoomFail();
		}
	} else if(room_id == 101) {
		try {
			int room_capacity = static_cast<int>(data[6]);
			Map map_type = static_cast<Map>(data[7]);
			while(!server.CreateRoom(game_type, room_id, room_capacity, map_type)); //��������ɹ������÷��������ȡ�÷����
			BOOST_LOG_TRIVIAL(info) << "���ID��" << u_id << "����" << room_id + 1 << "�ŷ��䣨" << "����������" << room_capacity << "��)";
			WriteRoomSuccess();//��ͻ��˷��ͷ�����Ϣ,�Լ�������Ϣ
			WriteHeartPackage();//��ʼ����������
		} catch(std::exception& e) {
			BOOST_LOG_TRIVIAL(warning) << "���ID��" << u_id << "�����˴������Ϸ���ͻ�û�п��з����ˣ�" << e.what();
			WriteRoomFail();
		}
	} else if(room_id == 102){//���ټ���
		try {
			while(!server.QuickEnter(game_type, room_id));//����ʧ��������ִ�п��ټ��룬ֱ������true
			BOOST_LOG_TRIVIAL(info) << "���ID��" << u_id << "ͨ�����ټ������" << room_id + 1 << "�ŷ���";
			WriteRoomSuccess();//��ͻ��˷��ͷ�����Ϣ,�Լ�������Ϣ
			WriteHeartPackage();//��ʼ����������
		} catch (std::exception& e) {
			BOOST_LOG_TRIVIAL(warning) << "���ID��" << u_id << "���ټ���ʧ�ܣ����з�������:" << e.what();
			WriteRoomFail();//û�п��з�����
		}
	}
}

void Session::WriteRoomSuccess() {
	char* data = new char[19];
	auto up_data = std::shared_ptr<char>(new char[4],[](char* c){delete[] c;});
	*up_data = Byte[update];//up_data[0]
	//auto uid = reinterpret_cast<unsigned short*>(up_data.get() + 2);//&up_data[2]
	//*uid = u_id;//����ҵ�id
	up_data.get()[2] = u_id >> 8;
	up_data.get()[3] = u_id & 0x00ff;
	Room& r = GetRoom();
	data[0] = Byte[success];
	data[1] = room_id + 1;
	data[2] = r.capacity;
	data[3] = r.map;
	int i = 0;

	while(r.listlock.test_and_set(std::memory_order_acquire));//����

	for(auto session : r.playerlist) {
		data[5 + i*2] = session->u_id >> 8;
		data[6 + i*2] = session->u_id & 0x00ff;
		++i;
	}
	data[4] = Byte[i];//����
	up_data.get()[1] = Byte[i+1];//���û��ڷ����еı��//up_data[1]
	for(auto session : r.playerlist) {
		//���Ӧ�ͻ��˵�д����д������Ϣ�����ö�Ӧ�ͻ��˵��߳�д��
		session->get_io_service().post(boost::bind(&Session::WriteBuf_Shared, session, up_data, 4));
	}
	r.playerlist.push_back(shared_from_this());//������б�������Լ���SessionPtr

	r.listlock.clear(std::memory_order_release);//���������������̶߳�ȡ�÷��������б�

	WriteBuf(data, 19);//���ͷ�����Ϣ
	if(i == r.capacity - 1) {//���պ��Ƿ����е����һ��
		for(auto session : r.playerlist) {
			//�򷿼������пͻ��˷�����Ϸ��ʼ��Ϣ
			session->get_io_service().post(boost::bind(&Session::WriteGameStart, session));
		}
		BOOST_LOG_TRIVIAL(info) << room_id + 1 << "�ŷ���������ʼ��Ϸ��";
		WriteGameOver();//�ɱ��̸߳���ִ����Ϸ��ʱ�������񣬵��ȷ�����Ϸ������Ϣ
	}
	delete []data;
}

void Session::WriteRoomFail() {
	char data = Byte[fail];
	WriteBuf(&data, 1);
}
void Session::WriteBuf(const void* data, std::size_t len) {
	bool emptybefore = true;
	if(!write_buf.IsEmptyBuf()) emptybefore = false;
	write_buf.Append(data, len);
	if(emptybefore) {//��д����֮ǰ����Ϊ�գ��������첽д
		Write();
	}
}

void Session::WriteBuf_Shared(std::shared_ptr<char> data_ptr, std::size_t len) {
	const void* data = data_ptr.get();
	WriteBuf(data, len);
}//data_ptrָ���ڴ��ڴ��ͷ�

void Session::WriteHeartPackage() {
	char data = Byte[beat];
	WriteBuf(&data, 1);
	if(!isclose) { //��socketû�жϿ�ʱ����ִ�д˶�ʱ���񣬷���share_ptr��ʹ�ñ������ܱ��ͷ�
		dtimer.expires_from_now(boost::posix_time::seconds(30));//�󶨱��̣߳�ÿ������ӷ�һ��
		dtimer.async_wait(boost::bind(&Session::WriteHeartPackage, shared_from_this()));
	}
}

void Session::WriteGameOver() {
	//ֻ�л���ģʽ��Ҫ��ʱ
	if(game_type == bonus) {
		gtimer.expires_from_now(posix_time::minutes(5));//5���Ӻ������Ϸ
		gtimer.async_wait(boost::bind(&Session::handle_close, shared_from_this()));
	}
}

//��ֱ�ӹر�socket�����ǵ�����˷����޷���дʱ���йرգ�ͬʱ������ָ�������Session���������ڣ���֤����ִ�������һ���첽���þ��
void Session::handle_close() {
	Room& r = GetRoom();
	auto over_data = std::shared_ptr<char>(new char(Byte[over]));
	for(auto session : r.playerlist) {
		//���Ӧ�ͻ��˵�д����д������Ϣ�����ö�Ӧ�ͻ��˵��߳�д��
		session->get_io_service().post(boost::bind(&Session::WriteBuf_Shared, session, over_data, 1));
	}
	BOOST_LOG_TRIVIAL(info) << room_id + 1 << "�ŷ�����Ϸ���������÷������";
	std::vector<boost::shared_ptr<Session>>().swap(r.playerlist);//��������б��ڵ����
	//���÷������
	r.capacity = 4;
	r.size = 0;
	r.map = map1;
	r.isempty = true;
	r.isfull = false;
}

void Session::WriteGameStart() {
	char* data = new char[17];
	Room& r = GetRoom();
	data[0] = Byte[start];
	int i = 0;
	for(auto session : r.playerlist) {
		//���ÿ����ҵĳ������ͺ���������
		data[++i] = session->car_type;
		data[++i] = session->car_img;
	}
	WriteBuf(data, 17);
	//��ʼ�첽������
	Read();
	delete []data;
}

void Session::Write() {
	if(isclose)
		return;
	async_write(socket, 
		write_buf.Data(), 
		boost::bind(&Session::handle_write, shared_from_this(),
		placeholders::error, placeholders::bytes_transferred)
		);
}

void Session::handle_write(const boost::system::error_code& err, size_t byte_transferred) {
	if(err) {
		BOOST_LOG_TRIVIAL(error) << "�����ID��" << u_id << "��������ʱ��������ѶϿ�����";
		Close();
		return;
	}
	write_buf.Consume(byte_transferred);
	if(!write_buf.IsEmptyBuf())
		Write();//��д�������ǿ�ʱ�����������첽д
}
void Session::Read() {
	async_read(socket,
		read_buf.GetTypeBuf(), //�ȶ������ֽ�
		boost::bind(&Session::handle_read_head, shared_from_this(),
		placeholders::error, placeholders::bytes_transferred)
		);
}

void Session::handle_read_head(const boost::system::error_code& err, size_t byte_transferred) {
	if(err) {
		BOOST_LOG_TRIVIAL(error) << "�����ID��" << u_id << "��ȡ����ʱ��������ѶϿ�����";
		Close();
		return;
	}
	read_buf.Retrive(byte_transferred);
	const char* data = read_buf.Peek();
	Type head_type = static_cast<Type>(*data);
	if(*data == Byte[operate]) { //������
		async_read(socket,
			read_buf.GetMsgBuf(operate), 
			boost::bind(&Session::handle_read, shared_from_this(),
			placeholders::error, placeholders::bytes_transferred, head_type)
			);
	} else if(*data == Byte[close]) {//�˳���
		async_read(socket,
			read_buf.GetMsgBuf(close), 
			boost::bind(&Session::handle_read, shared_from_this(),
			placeholders::error, placeholders::bytes_transferred, head_type)
			);
	} else {//���������������������
		read_buf.Consume(byte_transferred);
		Read();
		return;
	}
}

void Session::handle_read(const boost::system::error_code& err, size_t byte_transferred, Type t) {
	auto data = std::shared_ptr<char>(new char[byte_transferred + 1],[](char* c){delete[] c;});
	//data.get()[0] = Byte[t];
	read_buf.Retrive(byte_transferred);
	const char* transdata = read_buf.Peek();
	for(unsigned int i = 0; i < byte_transferred + 1; ++i) {
		data.get()[i] = transdata[i];
	}
	Room& r = GetRoom();
	for(auto session : r.playerlist) {
		session->get_io_service().post(boost::bind(&Session::WriteBuf_Shared, session, data, byte_transferred + 1));
	}
	Read();
	read_buf.Consume(byte_transferred + 1);
}

Room& Session::GetRoom(){
	switch(game_type) {
	case bonus:
		return server.bonus_room[room_id];
	case winner:
		return server.winner_room[room_id];
	case team://TODO
		;}
}