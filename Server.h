#ifndef _SERVER_H_
#define _SERVER_H_
#include <boost/smart_ptr.hpp>
#include <atomic>
#include <vector>
#include <array>
#include <iostream>
#include "IOServicePool.h"
#include "DataBuffer.h"
#include "Session.h"
#include "GameLog.h"
#define ROOM_CAPACITY 4
#define MAX_CAPACITY 8

class Session;

struct Room {
	typedef boost::shared_ptr<Session> Session_Ptr;
	//��������
	int capacity;
	//��ǰ��������
	int size;
	//�����ͼ
	Map map;
	//��ҳ�Ա�б�
	std::vector<Session_Ptr> playerlist;
	//�����жϷ���������������
	std::atomic_flag roomlock;
	//����ʵ�ַ��ʳ�Ա�б��������
	std::atomic_flag listlock;
	//�����жϷ����Ƿ�Ϊ��
	bool isempty;
	//�����жϷ����Ƿ���Ա
	bool isfull;
	//Ĭ�Ϲ��캯��
	Room():capacity(ROOM_CAPACITY),size(0),isempty(true),isfull(false),map(map1){
		roomlock.clear(std::memory_order_relaxed);
		listlock.clear(std::memory_order_relaxed);
	}
};

class Server {
public:
	typedef boost::asio::ip::tcp::acceptor acceptor_type;
	typedef boost::shared_ptr<Session> Session_Ptr;
	std::array<Room, 100> bonus_room;
	std::array<Room, 100> winner_room;
	IOServicePool ios_pool;
	acceptor_type acceptor;
	//�����˿ڼ������첽��������
	void start_accept();
	//accept���첽������
	void handle_accept(const boost::system::error_code& err, Session_Ptr session);
public:
	//���캯������nָ���߳���
	Server(unsigned short port, int n = 1);
	//����������
	inline void Start() { ios_pool.run();}
	//����ָ�����͵ķ��䣬�������͵Ĳ�����ֵΪ�����
	bool CreateRoom(Type t, int& room_id, int room_capacity, Map map_type);
	//����ָ�����ͺͷ���ŵķ���
	bool EnterRoom(Type t, int room_id);
	//���ټ��뷿��
	bool QuickEnter(Type t, int& room_id);

};
#endif