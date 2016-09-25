#include "networkWrapper.hpp"
#include "ball_generated.h"
#include "ballClient.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <functional>
#include <chrono>

std::mutex global_stream_lock;

const double Ball::scale = 0.75;

void Ball::bounce() {
	this->yVel = 10.0;
}

void Ball::tick() {
	this->y += this->yVel;
	this->yVel -= 1;
	if(this->y < 0) {
		this->y = 0;
		this->yVel = 0;
	}
}

std::string Ball::str() const {
	std::stringstream ss;
	ss << "("
		 << std::setw(6)
		 << std::setfill('0')
		 << std::setprecision(3)
		 << this->y
		 << ")";
	return ss.str();
}

void WorkerThread( std::shared_ptr< Hive > hive)
{
	global_stream_lock.lock();
	std::cout << "thread starting" << std::endl;
	global_stream_lock.unlock();
	hive->Run();

}

BallClient::BallClient()
	: isInit_(false) {
}

void BallClient::processUpdate(std::vector<uint8_t> & inBuffer,
	std::shared_ptr<BallConnection> connection) {

	auto bu = BallUpdate::GetUpdate(inBuffer.data());

	global_stream_lock.lock();
	std::cout << "[" << __FUNCTION__ << "]" << std::endl;
	global_stream_lock.unlock();

	if(bu->type() == BallUpdate::UpdateType::UpdateType_INIT) {
		init(bu->id()->c_str(),connection);
		global_stream_lock.lock();
		std::cout << "[" << __FUNCTION__ << "]"
							<< ": INIT" << std::endl;
		global_stream_lock.unlock();
	} else if(bu->type() == BallUpdate::UpdateType::UpdateType_BOUNCE) {
		if(id != bu->id()->c_str()
			&& otherBalls.find(bu->id()->c_str()) != otherBalls.end()) {
			otherBalls.find(bu->id()->c_str())->second.bounce();
		}
		global_stream_lock.lock();
		std::cout << "[" << __FUNCTION__ << "]"
							<< ": BOUNCE" << std::endl;
		global_stream_lock.unlock();
	} else if(bu->type() == BallUpdate::UpdateType::UpdateType_NEWBALL) {
		if(otherBalls.find(bu->id()->c_str()) == otherBalls.end()) {
			otherBalls[bu->id()->c_str()] = Ball();
		}
		global_stream_lock.lock();
		std::cout << "[" << __FUNCTION__ << "]"
							<< ": NEWBALL" << std::endl;
		global_stream_lock.unlock();
	} else if(bu->type() == BallUpdate::UpdateType::UpdateType_DELBALL) {
		if(otherBalls.find(bu->id()->c_str()) != otherBalls.end()) {
			otherBalls.erase(otherBalls.find(bu->id()->c_str()));
		}
		global_stream_lock.lock();
		std::cout << "[" << __FUNCTION__ << "]"
							<< ": DELBALL" << std::endl;
		global_stream_lock.unlock();
	}
}

bool BallClient::pollInput() {
	std::string input = "";
	std::cin.clear();
	std::cin >> input;

	if(input == "quit")
		return false;
	else{
		if(isInit()) {
			global_stream_lock.lock();
			std::cout << "BOUNCE!" << std::endl;
			global_stream_lock.unlock();
			this->yourBall.bounce();
			flatbuffers::FlatBufferBuilder builder;
			auto updateOffset = BallUpdate::CreateUpdate(builder,
				builder.CreateString(this->id),
				BallUpdate::UpdateType::UpdateType_BOUNCE);
			builder.Finish(updateOffset);
			std::vector<uint8_t> buf(builder.GetBufferPointer(),
				builder.GetBufferPointer() + builder.GetSize());
			connection->Send(buf);
		}
		return true;
	}
}

void BallClient::printBalls() {
	std::string out;
	yourBall.tick();
	out = yourBall.str();

	std::map<std::string, Ball>::iterator it,end;
	end = otherBalls.end();
	for (it = otherBalls.begin(); it != end; ++it) {
		it->second.tick();
    out += " " + it->second.str();
	}

	global_stream_lock.lock();
	std::cout << out << std::endl;
	global_stream_lock.unlock();
}

void BallClient::init(std::string id,
	std::shared_ptr<BallConnection> connection) {
	this->connection = connection;
	this->id = id;

	this->isInit_ = true;
}

bool BallClient::isInit() const {
	return isInit_;
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

	// std::string str = "GET / HTTP/1.0\r\n\r\n";

	// std::vector< uint8_t > request;
	// std::copy( str.begin(), str.end(), std::back_inserter( request ) );
	// Send( request );

	// BallUpdate::BallUpdate bu;
	// bu.set_type(BallUpdate::BallUpdate_Type_INIT);
	// SendUpdate(bu);
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
	std::cout << "[" << __PRETTY_FUNCTION__ << "] "
						<< buffer.size() << " bytes" << std::endl;
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


	ballClnt->processUpdate(buffer,
		std::dynamic_pointer_cast<BallConnection>(shared_from_this()));


	// Start the next receive
	Recv();
}

void BallConnection::OnTimer( const std::chrono::milliseconds & delta )
{
	// global_stream_lock.lock();
	// std::cout << "[" << __PRETTY_FUNCTION__ << "] " << delta << std::endl;
	// global_stream_lock.unlock();

	ballClnt->printBalls();
}

void BallConnection::OnError( const asio::error_code & error )
{
	global_stream_lock.lock();
	std::cout << "[" << __PRETTY_FUNCTION__ << "] " << error
						<< ": " << error.message() << std::endl;
	global_stream_lock.unlock();
}

BallConnection::BallConnection( std::shared_ptr<BallClient> ballClnt,
	std::shared_ptr< Hive > hive )
	: Connection( hive ), ballClnt(ballClnt)
{
}

BallConnection::~BallConnection()
{
}

std::shared_ptr<Connection> BallConnection::NewConnection () {
	return std::shared_ptr<BallConnection>(new BallConnection(
			this->ballClnt,this->GetHive()));
}

int main( int argc, char * argv[] )
{

	std::shared_ptr<BallClient> ballClnt( new BallClient() );

	std::shared_ptr< Hive > hive( new Hive() );

	std::shared_ptr< BallConnection > connection(
		new BallConnection( ballClnt,hive ) );
	connection->Connect( "localhost", 7777 );

	std::thread worker_thread(
		std::bind(&WorkerThread, hive));

	// initialize
	flatbuffers::FlatBufferBuilder builder;
	auto updateBuilder = BallUpdate::UpdateBuilder(builder);
	updateBuilder.add_type(BallUpdate::UpdateType::UpdateType_INIT);
	auto updateOffset = updateBuilder.Finish();
	builder.Finish(updateOffset);
	std::vector<uint8_t> buf(builder.GetBufferPointer(),
		builder.GetBufferPointer() + builder.GetSize());
	connection->Send(buf);

	while(!ballClnt->isInit()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	while (ballClnt->pollInput()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	hive->Stop();

	worker_thread.join();

	return 0;
}
