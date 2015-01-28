#ifndef _SESSION_H_
#define _SESSION_H_
#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include "DataBuffer.h"
#include <memory>
#include <fstream>



class Session {
public:
	typedef boost::asio::ip::tcp::socket socket_type;
	typedef boost::asio::io_service ios_type;
	//�����ķ�����
	//static Server* server;
	void init();
	//���캯��
	Session(ios_type& ios, int logno);
	//���socket
	inline socket_type& get_socket() {return socket;}
	//���io_service
	inline ios_type& get_io_service() {return socket.get_io_service();}
	//��ö�����
	inline DataBuffer& get_read_buf() {return read_buf;}
	//���д����
	inline DataBuffer& get_write_buf() {return write_buf;}
	//�ر�TCP����
	void Close();
	//��ȡ�����Ϣ��
	void ReadPlayerInfo();
	//��ҽ��뷿��ɹ������
	void WriteRoomSuccess();
	//����ҽ��뷿��ʧ�ܺ����
	void WriteRoomFail();
	//����ҷ���������
	void WriteHeartPackage();
	//����ҷ�����Ϸ��ʼ��Ϣ��
	void WriteGameStart();
	//����ҷ�����Ϸ������Ϣ��
	void WriteGameOver();
	//�򻺳���д����
	void WriteBuf(const void* data, std::size_t len);
	//�򻺳���д����,������ָ�뱣֤�������ڶ��̵߳�������ȷʹ��
	void WriteBuf_Shared(std::shared_ptr<char> data_ptr, std::size_t len);
	//�첽����д����������
	void Write();
	//�첽�����ݵ�������
	void Read();
	//��÷��������
	//Room& GetRoom();
	void WriteJob();
private:
	//�����Ϣ������
	//void handle_read_player(const boost::system::error_code& err,
	//	size_t byte_transferred);
	//��Ϸ��ʼ���ͷ������
	//void handle_read_head(const boost::system::error_code& err,
	//	size_t byte_transferred);
	//��Ϸ��ʼ���������
	void handle_read(const boost::system::error_code& err,
		size_t byte_transferred);
	//д������
	void handle_write(const boost::system::error_code& err,
		size_t byte_transferred);
	//���deadline_timer��ʱ�ر�socket
	//void handle_close();
	//asio��sokcet��װ
	socket_type socket;
	//������
	DataBuffer read_buf;
	//д����
	DataBuffer write_buf;
	//�����ж�socket�����Ƿ��Ѿ��Ͽ�
	bool isclose;
	//���ѡ�����Ϸ����
	Type game_type;
	//������ڷ���id
	int room_id;
	//��������ݿ��ж�Ӧ��id
	unsigned short u_id;
	//���ѡ��ĳ�������
	Car car_type;
	//���ѡ�����������
	CarImg car_img;
	//��ʱ������֤��session������������һֱ����
	boost::asio::deadline_timer dtimer;
	boost::asio::deadline_timer gtimer;
	std::ofstream ofs;
	int pid;
	
};
#endif