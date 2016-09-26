#include "ballServer.hpp"
#include <unistd.h>
#include <iostream>
#include <random>
#include <mutex>
#include <queue>
#include <thread>

std::mutex global_stream_lock;
std::mutex global_rng_lock;

void WorkerThread( std::shared_ptr< Hive > hive)
{
	global_stream_lock.lock();
	std::cout << "thread started" << std::endl;
	global_stream_lock.unlock();

	hive->Run();

	global_stream_lock.lock();
	std::cout << "thread ended" << std::endl;
	global_stream_lock.unlock();
}



std::string genIdentifier() {
	static const std::string alphanums =
		"0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const unsigned int idLen = 32;
	static std::mt19937 gen;
	static std::uniform_int_distribution<int> pick(0,alphanums.length()-1);
	global_rng_lock.lock();
	std::string id;
	for (unsigned int i = 0; i < idLen; i++) {
    id += alphanums[pick(gen)];
	}
	global_rng_lock.unlock();
	return id;
}

void BallServer::process(const bouncingBall::BallUpdate & bu,
	std::shared_ptr<BallConnection> connection) {

	global_stream_lock.lock();
	std::cout << "[" << __FUNCTION__ << "]" << std::endl;
	global_stream_lock.unlock();

	if(bu.type() == bouncingBall::BallUpdate_Type_INIT) {
		// create id
		std::string id = genIdentifier();
		std::cout << "New: " << id << " " << connection << std::endl;

		// initialize new ball and send INIT
		bouncingBall::BallUpdate buInit;
		buInit.set_type(bouncingBall::BallUpdate_Type_INIT);
		buInit.set_id(id);

		connection->SendUpdate(buInit);

		// notify others of the new ball and tell the new ball about existing balls
		bouncingBall::BallUpdate buNewBall;
		buNewBall.set_type(bouncingBall::BallUpdate_Type_NEWBALL);

		std::map<std::shared_ptr<BallConnection>,std::string>
			::const_iterator
			it,end;
		end = connToId.end();
		for (it = connToId.begin(); it != end; ++it) {
			// send new ball an existing ball
			buNewBall.set_id(it->second);
			connection->SendUpdate(buNewBall);

			// send existing ball the new ball
			buNewBall.set_id(id);
			it->first->SendUpdate(buNewBall);
		}

		// add ball to containers
		connToId[connection] = id;
		idToConn[id] = connection;

	}	else if(bu.type() == bouncingBall::BallUpdate_Type_BOUNCEBALL) {
		std::map<std::shared_ptr<BallConnection>,std::string >
			::const_iterator
			it,end;
		end = connToId.end();
		for (it = connToId.begin(); it != end; ++it) {
			if(it->second.compare(bu.id()))
				it->first->SendUpdate(bu);
		}
	} else if(bu.type() == bouncingBall::BallUpdate_Type_DELBALL) {
		std::string id = connToId.find(connection)->second;
		connToId.erase(connToId.find(connection));
		idToConn.erase(idToConn.find(id));

		bouncingBall::BallUpdate bu;
		bu.set_type(bouncingBall::BallUpdate_Type_DELBALL);
		bu.set_id(id);

		std::map<std::shared_ptr<BallConnection>,std::string>
			::const_iterator
			it,end;
		end = connToId.end();
		for (it = connToId.begin(); it != end; ++it) {
			it->first->SendUpdate(bu);
		}
	}
}

void BallConnection::SendUpdate( const bouncingBall::BallUpdate & bu)
{
	std::string buStr;
	bu.SerializeToString(&buStr);

	std::vector<uint8_t> buArr(buStr.begin(),buStr.end());
	Send(buArr);
}


void BallConnection::OnAccept( const std::string & host, uint16_t port )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] "
						<< host << ":" << port << std::endl;
	global_stream_lock.unlock();

	// Start the next receive
	Recv();
}


void BallConnection::OnConnect( const std::string & host, uint16_t port )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] "
						<< host << ":" << port << std::endl;
	global_stream_lock.unlock();

	// Start the next receive
	Recv();
}

void BallConnection::OnSend( const std::vector< uint8_t > & buffer )
{
	global_stream_lock.lock();
	// std::cout << "[" << __PRETTY_FUNCTION__ << "] "
	// 					<< buffer.size() << " bytes" << std::endl;
	// for( size_t x = 0; x < buffer.size(); ++x )
	// {
	// 	std::cout << std::hex << std::setfill( '0' ) <<
	// 		std::setw( 2 ) << (int)buffer[ x ] << " ";
	// 	if( ( x + 1 ) % 16 == 0 )
	// 	{
	// 		std::cout << std::endl;
	// 	}
	// }
	// std::cout << std::endl;
	global_stream_lock.unlock();
}

void BallConnection::OnRecv( std::vector< uint8_t > & buffer )
{

	global_stream_lock.lock();
	std::cout << "[" << __FUNCTION__ << "]" << std::endl;
	global_stream_lock.unlock();
	// global_stream_lock.lock();
	// std::cout << "[" << __PRETTY_FUNCTION__ << "] "
	// 					<< buffer.size() << " bytes" << std::endl;
	// for( size_t x = 0; x < buffer.size(); ++x )
	// {
	// 	std::cout << std::hex << std::setfill( '0' ) <<
	// 		std::setw( 2 ) << (int)buffer[ x ] << " ";
	// 	if( ( x + 1 ) % 16 == 0 )
	// 	{
	// 		std::cout << std::endl;
	// 	}
	// }
	// std::cout << std::endl;
	// global_stream_lock.unlock();

	bouncingBall::BallUpdate bu;
	bu.ParseFromString(std::string(buffer.begin(),buffer.end()));
	if(bu.has_type()) {
		ballSrv->process(bu,
			std::dynamic_pointer_cast<BallConnection>(shared_from_this()));
	}

	// Start the next receive
	Recv();
}

void BallConnection::OnTimer( const std::chrono::milliseconds & delta )
{
	// global_stream_lock.lock();
	// std::cout << "[" << __PRETTY_FUNCTION__ << "] " << delta << std::endl;
	// global_stream_lock.unlock();
}

void BallConnection::OnError( const asio::error_code & error )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] " << error
						<< ": " << error.message() << std::endl;
	global_stream_lock.unlock();

	if(error != asio::error::operation_aborted
		&& error) {
		bouncingBall::BallUpdate bu;
		bu.set_type(bouncingBall::BallUpdate_Type_DELBALL);

		ballSrv->process(bu,
			std::dynamic_pointer_cast<BallConnection>(shared_from_this()));
	}
}

BallConnection::BallConnection( std::shared_ptr<BallServer> ballSrv,
	std::shared_ptr< Hive > hive )
	: Connection( hive ), ballSrv(ballSrv)
{
}

BallConnection::~BallConnection()
{
}

std::shared_ptr<Connection> BallConnection::NewConnection(){
	return std::shared_ptr<BallConnection>(
		new BallConnection(this->ballSrv,this->GetHive()));
}

bool BallAcceptor::OnAccept( std::shared_ptr< Connection > connection,
	const std::string & host, uint16_t port )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] "
						<< host << ":" << port << std::endl;
	global_stream_lock.unlock();

	return true;
}

void BallAcceptor::OnTimer( const std::chrono::milliseconds & delta )
{
	// global_stream_lock.lock();
	// std::cout << "[" << __PRETTY_FUNCTION__ << "] " << delta << std::endl;
	// std::map<std::shared_ptr<BallConnection>,std::string >
	// 	::const_iterator
	// 	it,end;
	// end = ballSrv->connToId.end();
	// for (it = ballSrv->connToId.begin(); it != end; ++it) {
	// 	std::cout << it->first << " : " << it->second << std::endl;
	// }
	// global_stream_lock.unlock();
}

void BallAcceptor::OnError( const asio::error_code & error )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] " << error << std::endl;
	global_stream_lock.unlock();
}
BallAcceptor::BallAcceptor( std::shared_ptr<BallServer> ballSrv,
	std::shared_ptr< Hive > hive )
	: Acceptor( hive ), ballSrv(ballSrv)
{
}

BallAcceptor::~BallAcceptor()
{
}

int main( int argc, char * argv[] )
{
	std::shared_ptr<BallServer> ballSrv( new BallServer() );

	std::shared_ptr< Hive > hive( new Hive() );

	std::shared_ptr< BallAcceptor > acceptor(
		new BallAcceptor( ballSrv, hive ) );
	acceptor->Listen( "0.0.0.0", 7777 );

	std::shared_ptr< BallConnection > connection(
		new BallConnection(ballSrv,hive));
	acceptor->Accept( connection );

	std::thread worker_thread(
		std::bind(&WorkerThread, hive));

	std::string input;
	while(input.compare("quit")) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::cin.clear();
		std::cin >> input;
	}

	hive->Stop();

	worker_thread.join();

	return 0;
}
