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

			//вывод информации о сообщении (его id и размер в байтах);
			template <typename T>
			friend  std::ostream& operator << (std::ostream& os, const Message<T>& message)
			{
				os << "Id = " << message.header.id << " Size = " << message.header.size << std::endl;
				return os;
			}

			//добавление данных в вектор самого сообщения (добавление в конец)
			//msg << 7.5 (то есть, мы записывает float в сообщение)
			template <typename DataType>
			friend  Message<T>& operator << (Message<T>& msg, const DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");
				
				size_t i = msg.body.size(); // получаем текущий размер сообщения
				
				msg.body.resize(msg.body.size() + sizeof(DataType)); //увеличиваем размер вектора , чтобы вписать данные
				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));// записываем в вектор данные

				msg.header.size = msg.size(); // меняем размер всего сообщения
				return msg;
			}
			//извлечение данных из вектора самого сообщения (извлечение с конца)
			// float a; msg >> a (то есть, мы извлекаем последний данные из вектора и записываем их например во float)
			template <typename DataType>
			friend  Message<T>& operator >> (Message<T>& msg, const DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

				size_t i = msg.body.size() - sizeof(DataType);  // получаем новый размер вектора

				std::memcpy((void*)&data, msg.body.data() + i, sizeof(DataType)); //записываем в переменную нужно количество байт из вектора

				msg.body.resize(i); // меняем размер вектора
				msg.header.size = msg.size();  // меняем размер всего сообщения
				
				return msg;
			}
		};

		template<typename T>
		class Connection;



		template<typename T>
		struct OwnedMessage
		{
			//просто инкапсулируем сообщение в данный класс, чтобы у нас был указатель на соединение.
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