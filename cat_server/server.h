#pragma once
#include "../tlv_parser/tlv_parser.h"
#include "../tlv_parser/tlv_decoder.h"
#include "../filters/file_filter.hpp"
#include <boost/config.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "connection.h"

struct Server
{
	Server (boost::asio::io_service & io_service, boost::asio::ip::tcp::endpoint const & endpoint)
		: m_io_service(io_service)
		, m_acceptor(io_service, endpoint)
	{
		start_accept();
	}

	void start_accept ()
	{
		connection_ptr_t new_session(new Connection(m_io_service));
		m_acceptor.async_accept(new_session->socket(),
				boost::bind(&Server::handle_accept, this, new_session, boost::asio::placeholders::error));
	}

	void handle_accept (connection_ptr_t session, boost::system::error_code const & error)
	{
		if (!error)
			session->start();

		start_accept();
	}

private:
	boost::asio::io_service & m_io_service;
	boost::asio::ip::tcp::acceptor m_acceptor;
};

typedef boost::shared_ptr<Server> server_ptr_t;
