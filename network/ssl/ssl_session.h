#ifndef _SSL_SESSION_
#define _SSL_SESSION_


#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/asio/ssl.hpp>

#include "ssl_message.h"
#include "ThreadSafeQueue/job_queue.h"

#include "network/isession.h"

class IMessage;


class ssl_session : public boost::enable_shared_from_this<ssl_session>, public ISession
{
public:
	//typedef boost::asio::ip::tcp::socket socket_type;
	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_type;
	typedef boost::asio::io_service ios_type;

	typedef ios_type::strand strand_type;
	typedef job_queue<IMessage*> queue_type;
	typedef boost::object_pool<IMessage*> object_pool_type;

private:
	socket_type socket_;
	strand_type strand_;
	queue_type& queue_;
	static object_pool_type msg_pool_;


public:
	ssl_session(ios_type& ios, queue_type& q,boost::asio::ssl::context& context);

	socket_type::lowest_layer_type& socket()
	{
		//return socket_;
		return socket_.lowest_layer();

	}

	ios_type& io_service();

	void start();
	void close();
	

	bool verify_certificate(bool preverified,      boost::asio::ssl::verify_context& ctx)
	{
		// The verify callback can be used to check whether the certificate that is
		// being presented is valid for the peer. For example, RFC 2818 describes
		// the steps involved in doing this for HTTPS. Consult the OpenSSL
		// documentation for more details. Note that the callback is called once
		// for each certificate in the certificate chain, starting from the root
		// certificate authority.

		// In this example we will simply print the certificate's subject name.
		char subject_name[256];
		X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
		OutputDebugString("Verifying " );
		OutputDebugString(subject_name);
		OutputDebugString("\n");

		return preverified;
	}

public:
	IMessage* create_request();
	//void read(ssl_request_ptr req);
	void read();
	void handle_handshake(const boost::system::error_code& error);

	void handle_read_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req);
	void handle_read_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* req);

	void write(IMessage* resp);
	void handle_write_head(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp);
	void handle_write_msg(const boost::system::error_code& error, size_t bytes_transferred, IMessage* resp);
};

typedef boost::shared_ptr<ssl_session> ssl_session_ptr;

#endif // _TCP_SESSION_

