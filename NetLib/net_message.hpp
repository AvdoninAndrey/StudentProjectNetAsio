#pragma once
#include "net_common.hpp"

namespace olc
{
	namespace net
	{
		template <typename T>
		struct MessageHeader
		{
			T id{};
			uint32_t size = 0;
		};

		template <typename T>
		struct Message
		{
			MessageHeader<T> header;
			std::vector<uint8_t> body;

			size_t size() const
			{
				return sizeof(MessageHeader<T>) + body.size();
			}

			//����� ���������� � ��������� (��� id � ������ � ������);
			template <typename T>
			friend  std::ostream& operator << (std::ostream& os, const Message<T>& message)
			{
				os << "Id = " << message.header.id << " Size = " << message.header.size << std::endl;
				return os;
			}

			//���������� ������ � ������ ������ ��������� (���������� � �����)
			//msg << 7.5 (�� ����, �� ���������� float � ���������)
			template <typename DataType>
			friend  Message<T>& operator << (Message<T>& msg, const DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");
				
				size_t i = msg.body.size(); // �������� ������� ������ ���������
				
				msg.body.resize(msg.body.size() + sizeof(DataType)); //����������� ������ ������� , ����� ������� ������
				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));// ���������� � ������ ������

				msg.header.size = msg.size(); // ������ ������ ����� ���������
				return msg;
			}
			//���������� ������ �� ������� ������ ��������� (���������� � �����)
			// float a; msg >> a (�� ����, �� ��������� ��������� ������ �� ������� � ���������� �� �������� �� float)
			template <typename DataType>
			friend  Message<T>& operator >> (Message<T>& msg, const DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

				size_t i = msg.body.size() - sizeof(DataType);  // �������� ����� ������ �������

				std::memcpy((void*)&data, msg.body.data() + i, sizeof(DataType)); //���������� � ���������� ����� ���������� ���� �� �������

				msg.body.resize(i); // ������ ������ �������
				msg.header.size = msg.size();  // ������ ������ ����� ���������
				
				return msg;
			}
		};

		template<typename T>
		class Connection;



		template<typename T>
		struct OwnedMessage
		{
			//������ ������������� ��������� � ������ �����, ����� � ��� ��� ��������� �� ����������.
			std::shared_ptr<Connection<T>> remote = nullptr;
			Message<T> msg;

			friend std::ostream& operator << (std::ostream& os, const OwnedMessage<T>& msg)
			{
				os << msg.msg;
				return os;
			}
		};



	}
}