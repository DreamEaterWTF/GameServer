#include "Session.h"
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <cassert>
#include <sstream>
#include <boost/date_time.hpp>

using namespace boost;
using namespace boost::asio;

//����ȷ�����͵��ֽ�
char Byte[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

//�ǳ�����̬��Ա���������������ʼ��
//Server* Session::server = nullptr;
std::string timestamp();

Session::Session(ios_type& ios, int logno) : pid(logno + 1), socket(ios), isclose(false), dtimer(get_io_service()), gtimer(get_io_service(), boost::posix_time::seconds(10)){
}
void Session::init() {
	std::stringstream ss;
	ss << "./log/" << pid - 1 << ".log";
	ofs.open(ss.str());
	ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8384);
	socket.connect(ep);
	gtimer.async_wait(boost::bind(&Session::WriteJob, this));
}
void Session::Close() {
	boost::system::error_code ignored_ec;
	socket.shutdown(ip::tcp::socket::shutdown_both, ignored_ec);
	socket.close(ignored_ec);
	isclose = true;
	if(ignored_ec) {
		std::cout<<ignored_ec.message()<<std::endl;
	}
}

//void Session::ReadPlayerInfo() {
//	async_read(socket, 
//		read_buf.GetMsgBuf(bonus),            //������Ϸ�������Ϣ�ֽ���һ�£���������bonus��������������
//		boost::bind(&Session::handle_read_player, shared_from_this(),
//		placeholders::error, placeholders::bytes_transferred)
//		);
//}

//void Session::handle_read_player (const boost::system::error_code& err, size_t byte_transferred) {
//	if(err) {
//		std::cout<<err.message()<<std::endl;
//		Close();
//		return;
//	}
//	//TODO Log
//	read_buf.Retrive(byte_transferred);
//	const char* data = read_buf.Peek();
//	game_type = static_cast<Type>(data[0]);
//	u_id = *reinterpret_cast<const unsigned short*>(data + 1);
//	car_type = static_cast<Car>(data[3]);
//	car_img = static_cast<CarImg>(data[4]);
//	room_id = static_cast<int>(data[5]);
//	read_buf.Consume(byte_transferred);
//	if(room_id < 101 && room_id > 0) {
//		--room_id;
//		if(server->EnterRoom(game_type, room_id)) {//���뷿��ɹ�
//			WriteRoomSuccess();//��ͻ��˷��ͷ�����Ϣ,�Լ�������Ϣ
//			WriteHeartPackage();//��ʼ����������
//		} else {
//			WriteRoomFail();//��ͻ��˷��ͽ��뷿��ʧ����Ϣ
//		}
//	} else if(room_id == 101) {
//		int room_capacity = static_cast<int>(data[6]);
//		Map map_type = static_cast<Map>(data[7]);
//		if(server->CreateRoom(game_type, room_id, room_capacity, map_type)) {//��������ɹ������÷��������ȡ�÷����
//			WriteRoomSuccess();//��ͻ��˷��ͷ�����Ϣ,�Լ�������Ϣ
//			WriteHeartPackage();//��ʼ����������
//		} else {
//			WriteRoomFail();//��ͻ��˷��ͽ��뷿��ʧ����Ϣ
//		}
//	} else if(room_id == 102){//���ټ���
//		try {
//			while(!server->QuickEnter(game_type, room_id));//����ʧ��������ִ�п��ټ��룬ֱ������true
//			WriteRoomSuccess();//��ͻ��˷��ͷ�����Ϣ,�Լ�������Ϣ
//			WriteHeartPackage();//��ʼ����������
//		} catch (std::exception& e) {
//			std::cout<<e.what();
//			WriteRoomFail();//û�п��з�����
//		}
//	}
//}

//void Session::WriteRoomSuccess() {
//	char* data = new char[19];
//	auto up_data = std::shared_ptr<char>(new char[4],[](char* c){delete[] c;});
//	*up_data = Byte[update];//up_data[0]
//	auto uid = reinterpret_cast<unsigned short*>(up_data.get() + 2);//&up_data[2]
//	*uid = u_id;//����ҵ�id
//	Room& r = GetRoom();
//	data[0] = Byte[success];
//	data[1] = room_id + 1;
//	data[2] = r.capacity;
//	data[3] = r.map;
//	int i = 0;
//
//	while(r.listlock.test_and_set(std::memory_order_acquire));//����
//
//	for(auto session : r.playerlist) {
//		auto uid =reinterpret_cast<unsigned short*>(data+5+i*2);
//		*uid = session->u_id;//������ÿλ��ҵ�id
//		++i;
//	}
//	data[4] = Byte[i];//����
//	up_data.get()[1] = Byte[i+1];//���û��ڷ����еı��//up_data[1]
//	for(auto session : r.playerlist) {
//		//���Ӧ�ͻ��˵�д����д������Ϣ�����ö�Ӧ�ͻ��˵��߳�д��
//		session->get_io_service().post(boost::bind(&Session::WriteBuf_Shared, session, up_data, 4));
//	}
//	r.playerlist.push_back(shared_from_this());//������б�������Լ���SessionPtr
//
//	r.listlock.clear(std::memory_order_release);//���������������̶߳�ȡ�÷��������б�
//
//	WriteBuf(data, 19);//���ͷ�����Ϣ
//	if(i == r.capacity - 1) {//���պ��Ƿ����е����һ��
//		for(auto session : r.playerlist) {
//			//�򷿼������пͻ��˷�����Ϸ��ʼ��Ϣ
//			session->get_io_service().post(boost::bind(&Session::WriteGameStart, session));
//		}
//		WriteGameOver();//�ɱ��̸߳���ִ����Ϸ��ʱ�������񣬵��ȷ�����Ϸ������Ϣ
//	}
//	delete []data;
//}

//void Session::WriteRoomFail() {
//	char data = Byte[fail];
//	WriteBuf(&data, 1);
//}
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
		dtimer.async_wait(boost::bind(&Session::WriteHeartPackage, this));
	}
}

void Session::WriteJob() {
	char* data = new char[3];
	data[0] = 9;
	data[1] = pid%8? pid%8 : 8;
	//srand(time(nullptr));
	data[2] = rand() % 127;
	WriteBuf(data, 3);
	ofs << timestamp() << " ������Ϣ: ";
	for(size_t i = 0; i < 3; ++i) {
		ofs << (int)data[i] << "\t|";
	}
	ofs << std::endl;
	if(!isclose) { //��socketû�жϿ�ʱ����ִ�д˶�ʱ���񣬷���share_ptr��ʹ�ñ������ܱ��ͷ�
		gtimer.expires_from_now(boost::posix_time::milliseconds(500));//�󶨱��̣߳�ÿ�����뷢һ��
		gtimer.async_wait(boost::bind(&Session::WriteJob, this));
	}
	delete[] data;
}

//void Session::WriteGameOver() {
//	//ֻ�л���ģʽ��Ҫ��ʱ
//	if(game_type == bonus) {
//		gtimer.expires_from_now(posix_time::minutes(5));//5���Ӻ������Ϸ
//		gtimer.async_wait(boost::bind(&Session::handle_close, shared_from_this()));
//	}
//}

////��ֱ�ӹر�socket�����ǵ�����˷����޷���дʱ���йرգ�ͬʱ������ָ�������Session���������ڣ���֤����ִ�������һ���첽���þ��
//void Session::handle_close() {
//	Room& r = GetRoom();
//	auto over_data = std::shared_ptr<char>(new char(Byte[over]));
//	for(auto session : r.playerlist) {
//		//���Ӧ�ͻ��˵�д����д������Ϣ�����ö�Ӧ�ͻ��˵��߳�д��
//		session->get_io_service().post(boost::bind(&Session::WriteBuf_Shared, session, over_data, 1));
//	}
//	std::vector<boost::shared_ptr<Session>>().swap(r.playerlist);//��������б��ڵ����
//	//���÷������
//	r.capacity = 4;
//	r.size = 0;
//	r.map = map1;
//	r.isempty = true;
//	r.isfull = false;
//}

//void Session::WriteGameStart() {
//	char* data = new char[17];
//	Room& r = GetRoom();
//	data[0] = Byte[start];
//	int i = 0;
//	for(auto session : r.playerlist) {
//		//���ÿ����ҵĳ������ͺ���������
//		data[++i] = session->car_type;
//		data[++i] = session->car_img;
//	}
//	WriteBuf(data, 17);
//	//��ʼ�첽������
//	Read();
//	delete []data;
//}

void Session::Write() {
	if(isclose)
		return;
	async_write(socket, 
		write_buf.Data(), 
		boost::bind(&Session::handle_write, this,
		placeholders::error, placeholders::bytes_transferred)
		);
}

void Session::handle_write(const boost::system::error_code& err, size_t byte_transferred) {
	if(err) {
		Close();
		return;
	}
	write_buf.Consume(byte_transferred);
	if(!write_buf.IsEmptyBuf())
		Write();//��д�������ǿ�ʱ�����������첽д
}
void Session::Read() {
	socket.async_read_some(
		read_buf.prepare(), 
		boost::bind(&Session::handle_read, this,
		placeholders::error, placeholders::bytes_transferred)
		);
}

//void Session::handle_read_head(const boost::system::error_code& err, size_t byte_transferred) {
//	if(err) {
//		Close();
//		return;
//	}
//	read_buf.Retrive(byte_transferred);
//	const char* data = read_buf.Peek();
//	Type head_type = static_cast<Type>(*data);
//	if(*data == Byte[operate]) { //������
//		async_read(socket,
//			read_buf.GetMsgBuf(operate), 
//			boost::bind(&Session::handle_read, shared_from_this(),
//			placeholders::error, placeholders::bytes_transferred, head_type)
//			);
//	} else if(*data == Byte[close]) {//�˳���
//		async_read(socket,
//			read_buf.GetMsgBuf(close), 
//			boost::bind(&Session::handle_read, shared_from_this(),
//			placeholders::error, placeholders::bytes_transferred, head_type)
//			);
//	} else {//���������������������
//		read_buf.Consume(byte_transferred);
//		Read();
//		return;
//	}
//}


void Session::handle_read(const boost::system::error_code& err, size_t byte_transferred) {
	if(err) {
		Close();
		return;
	}
	ofs << timestamp() << " �յ���Ϣ: ";
	const char* rd = read_buf.Peek();
	for(size_t i = 0; i < byte_transferred; ++i) {
		ofs << (int)rd[i] << "\t|";
	}
	ofs << std::endl;
	Read();
	read_buf.Consume(byte_transferred + 1);
}

//Room& Session::GetRoom(){
//	switch(game_type) {
//	case bonus:
//		return server->bonus_room[room_id];
//	case winner:
//		return server->winner_room[room_id];
//	case team://TODO
//	default:
//		throw std::invalid_argument("wrong type of game");
//	}
//}