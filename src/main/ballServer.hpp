#ifndef BALL_SERVER_HPP
#define BALL_SERVER_HPP

#include "networkWrapper.hpp"
#include "ball.pb.h"
#include <queue>


// random identifier
std::string genIdentifier();

class BallServer;
class BallConnection;
class BallAcceptor;

class BallServer {
public:
	void process(const bouncingBall::BallUpdate & bu,
		boost::shared_ptr<BallConnection> connection);

// private:
public:
	// connection containers
	std::map<boost::shared_ptr<BallConnection>,
					 std::string> connToId;
	std::map<std::string,
					 boost::shared_ptr<BallConnection> > idToConn;
};


class BallConnection : public Connection
{
private:
	boost::shared_ptr<BallServer> ballSrv;

	void OnAccept( const std::string & host, uint16_t port );

	void OnConnect( const std::string & host, uint16_t port );

	void OnSend( const std::vector< uint8_t > & buffer );

	void OnRecv( std::vector< uint8_t > & buffer );

	void OnTimer( const boost::posix_time::time_duration & delta );

	void OnError( const boost::system::error_code & error );

public:
	BallConnection( boost::shared_ptr<BallServer> ballSrv,
		boost::shared_ptr< Hive > hive );


	~BallConnection();

	boost::shared_ptr<Connection> NewConnection();

	void SendUpdate(const bouncingBall::BallUpdate & letter);
};

class BallAcceptor : public Acceptor
{
private:
	boost::shared_ptr<BallServer> ballSrv;

	bool OnAccept( boost::shared_ptr< Connection > connection,
		const std::string & host, uint16_t port );

	void OnTimer( const boost::posix_time::time_duration & delta );

	void OnError( const boost::system::error_code & error );

public:
	BallAcceptor( boost::shared_ptr<BallServer> ballSrv,
		boost::shared_ptr< Hive > hive );

	~BallAcceptor();
};

#endif
