#pragma once
#include <google/tagged_tuple.hpp>

#define ___message_field___  google::internal_tagged_tuple::get
#define _message_field_(field) google::internal_tagged_tuple::tag<##field##>

namespace gologin
{
	namespace messages
	{
		namespace commands
		{
			constexpr std::string_view hello{"HELLO"};
			constexpr std::string_view auth{"login"};
			constexpr std::string_view msg{"message"};
			constexpr std::string_view msg_reply{"message_reply"};
			constexpr std::string_view ping{"ping"};
			constexpr std::string_view ping_reply{"ping_reply"};
			constexpr std::string_view bye{"logout"};
			constexpr std::string_view bye_reply{"logout"};
		};

		namespace client
		{
			using header =
					google::tagged_tuple<
					google::member<"id", std::uint64_t>, 
					google::member<"command", std::string>>;

			namespace hello
			{
				using request =
					google::tagged_tuple<
					google::member<"id", std::uint64_t>, 
					google::member<"command", std::string>>;
			}

			namespace bye
			{
				using request =
					google::tagged_tuple<
					google::member<"id", std::uint64_t>, 
					google::member<"command", std::string>,
					google::member<"session", std::string>>;
			}

			namespace ping
			{
				using request =
					google::tagged_tuple<
					google::member<"id", std::uint64_t>, 
					google::member<"command", std::string>,
					google::member<"session", std::string>>;
			}

			namespace auth
			{
				using request =
					google::tagged_tuple<
					google::member<"id", std::uint64_t>, 
					google::member<"command", std::string>,
					google::member<"login", std::string>,
					google::member<"password", std::string>>;
			}

			namespace msg
			{
				using request =
					google::tagged_tuple<
					google::member<"id", std::uint64_t>, 
					google::member<"command", std::string>,
					google::member<"body", std::string>,
					google::member<"session", std::string>>;

				using response =
						google::tagged_tuple<
						google::member<"id", std::uint64_t>, 
						google::member<"command", std::string>,
						google::member<"status", std::string>,
						google::member<"client_id", std::string>,
						google::member<"message", std::string>>;
			}
		}

		namespace server
		{
			using header =
					google::tagged_tuple<
					google::member<"id", std::uint64_t>, 
					google::member<"command", std::string>>;

			namespace hello
			{
				using response =
					google::tagged_tuple<
					google::member<"id", std::uint64_t>, 
					google::member<"command", std::string>,
					google::member<"auth_method", std::string>>;
			}

			namespace bye
			{
				using response =
					google::tagged_tuple<
					google::member<"id", std::uint64_t>, 
					google::member<"command", std::string>,
					google::member<"status", std::string>>;
			}

			namespace ping
			{
				using response =
						google::tagged_tuple<
						google::member<"id", std::uint64_t>, 
						google::member<"command", std::string>,
						google::member<"status", std::string>,
						google::member<"message", std::string>>;
			}

			namespace auth
			{
				using response =
						google::tagged_tuple<
						google::member<"id", std::uint64_t>, 
						google::member<"command", std::string>,
						google::member<"status", std::string>,
						google::member<"session", std::string>,
						google::member<"message", std::string>>;
			}

			namespace msg
			{
				using request =
						google::tagged_tuple<
						google::member<"id", std::uint64_t>, 
						google::member<"command", std::string>,
						google::member<"body", std::string>,
						google::member<"sender_login", std::string>,
						google::member<"session", std::string>>;

				using response =
						google::tagged_tuple<
						google::member<"id", std::uint64_t>, 
						google::member<"command", std::string>,
						google::member<"status", std::string>,
						google::member<"client_id", std::string>,
						google::member<"message", std::string>>;
			}
		}
	}
}