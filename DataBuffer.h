#ifndef _DATA_BUFFER_H_
#define _DATA_BUFFER_H_
#include <boost/asio.hpp>
#include <boost/config/suffix.hpp>
#include <boost/cast.hpp>

enum Type {beat, bonus, winner, team, success, fail, update, start, over, operate, close};
enum Car {car1, car2, car3};
enum CarImg {img1, img2, img3};
enum Map {map1,map2,map3};

class DataBuffer {
public:
	typedef std::size_t size_type;
	typedef boost::asio::streambuf streambuf_type;
	typedef streambuf_type::const_buffers_type const_buffers_type;
	typedef streambuf_type::mutable_buffers_type mutable_buffers_type;

private:
	//��ͷ�����ֽڴ�С
	static const int TYPE_SIZE = 1;
	//asio������
	streambuf_type m_buf;
public:
	//���������ֽڴ�С�Ļ��������ڽ�������
	inline mutable_buffers_type GetTypeBuf() { return m_buf.prepare(1);}
	//���ض�Ӧ�����͵���Ϣ��Ҫ�ֽڴ�С�Ļ��������ڽ�������
	mutable_buffers_type GetMsgBuf(Type t);
	//��ȡ���յ����ֽ�
	inline void Retrive(size_type n) {m_buf.commit(n);}
	//���ص�ǰ�������Ƿ�Ϊ��
	inline bool IsEmptyBuf() {return (m_buf.size() == 0 ? true : false);}
	//�鿴�ɶ����ֽ�
	inline const char* Peek() const {return boost::asio::buffer_cast<const char*>(m_buf.data());}
	//������ڷ������ݵĻ�����
	inline const_buffers_type Data() const {return m_buf.data();}
	//д���ݵ�������
	inline void Append(const void* data, size_type len) {m_buf.sputn(static_cast<const char*>(data),boost::numeric_cast<std::streamsize>(len));}
	//ָʾ������������n���ֽ�
	inline void Consume(size_type n){ m_buf.consume(n);}

};
#endif