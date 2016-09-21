#ifndef BALL_CLIENT_HPP
#define BALL_CLIENT_HPP

#include "networkWrapper.hpp"
#include "ball.pb.h"
#include <queue>

class Ball;
class BallClient;
class BallConnection;
class BallAcceptor;

class Ball {
private:
	double y; // y location
	double yVel; // y velocity

	const static double scale;

public:
	void bounce();

	void tick();

	std::string str() const;
};

class BallClient {
public:

	BallClient();

	void processUpdate(bouncingBall::BallUpdate bu,
		boost::shared_ptr<BallConnection> connection);

	void printBalls();

	void init(const std::string id,
		boost::shared_ptr<BallConnection> connection);

	bool isInit() const;

	bool pollInput();

	// void newBall(const std::string id);
	// void delBall(const std::string id);

private:
	boost::shared_ptr<BallConnection> connection;

	bool isInit_;
	std::string id;

	Ball yourBall;
	std::map<std::string, Ball> otherBalls;
};


class BallConnection : public Connection
{
private:
	boost::shared_ptr<BallClient> ballClnt;

	void OnAccept( const std::string & host, uint16_t port );

	void OnConnect( const std::string & host, uint16_t port );

	void OnSend( const std::vector< uint8_t > & buffer );

	void OnRecv( std::vector< uint8_t > & buffer );

	void OnTimer( const boost::posix_time::time_duration & delta );

	void OnError( const boost::system::error_code & error );

public:
	BallConnection( boost::shared_ptr<BallClient> ballClnt,
		boost::shared_ptr< Hive > hive );

	~BallConnection();

	boost::shared_ptr<Connection> NewConnection();

	void SendUpdate(const bouncingBall::BallUpdate & bu);
};

#endif
