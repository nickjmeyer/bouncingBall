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
	Ball();

	void bounce();

	void tick();

	std::string str() const;
};

class BallClient {
public:

	BallClient();

	void processUpdate(bouncingBall::BallUpdate bu,
		std::shared_ptr<BallConnection> connection);

	void printBalls();

	void init(const std::string id,
		std::shared_ptr<BallConnection> connection);

	bool isInit() const;

	bool pollInput();

	// void newBall(const std::string id);
	// void delBall(const std::string id);

private:
	std::shared_ptr<BallConnection> connection;

	bool isInit_;
	std::string id;

	Ball yourBall;
	std::map<std::string, Ball> otherBalls;
};


class BallConnection : public Connection
{
private:
	std::shared_ptr<BallClient> ballClnt;

	void OnAccept( const std::string & host, uint16_t port );

	void OnConnect( const std::string & host, uint16_t port );

	void OnSend( const std::vector< uint8_t > & buffer );

	void OnRecv( std::vector< uint8_t > & buffer );

	void OnTimer( const std::chrono::milliseconds & delta );

	void OnError( const asio::error_code & error );

public:
	BallConnection( std::shared_ptr<BallClient> ballClnt,
		std::shared_ptr< Hive > hive );

	~BallConnection();

	std::shared_ptr<Connection> NewConnection();

	void SendUpdate(const bouncingBall::BallUpdate & bu);
};

#endif
