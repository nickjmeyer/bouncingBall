#ifndef BALL_SERVER_HPP
#define BALL_SERVER_HPP

#include "networkWrapper.hpp"
#include "ball.pb.h"
#include <queue>
#include <asio/error.hpp>


// random identifier
std::string genIdentifier();

class BallServer;
class BallConnection;
class BallAcceptor;

class BallServer {
public:
	void process(const bouncingBall::BallUpdate & bu,
		std::shared_ptr<BallConnection> connection);

// private:
public:
	// connection containers
	std::map<std::shared_ptr<BallConnection>,
					 std::string> connToId;
	std::map<std::string,
					 std::shared_ptr<BallConnection> > idToConn;
};


class BallConnection : public Connection
{
private:
	std::shared_ptr<BallServer> ballSrv;

	void OnAccept( const std::string & host, uint16_t port );

	void OnConnect( const std::string & host, uint16_t port );

	void OnSend( const std::vector< uint8_t > & buffer );

	void OnRecv( std::vector< uint8_t > & buffer );

	void OnTimer( const std::chrono::milliseconds & delta );

	void OnError( const asio::error_code & error );

public:
	BallConnection( std::shared_ptr<BallServer> ballSrv,
		std::shared_ptr< Hive > hive );


	~BallConnection();

	std::shared_ptr<Connection> NewConnection();

	void SendUpdate(const bouncingBall::BallUpdate & letter);
};

class BallAcceptor : public Acceptor
{
private:
	std::shared_ptr<BallServer> ballSrv;

	bool OnAccept( std::shared_ptr< Connection > connection,
		const std::string & host, uint16_t port );

	void OnTimer( const std::chrono::milliseconds & delta );

	void OnError( const asio::error_code & error );

public:
	BallAcceptor( std::shared_ptr<BallServer> ballSrv,
		std::shared_ptr< Hive > hive );

	~BallAcceptor();
};

#endif
