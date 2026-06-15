#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

using std::cout;
using std::cin;
using std::endl;
using namespace std::chrono_literals;

bool finish = false;
std::mutex mutex;

void Plus()
{
	while (!finish)
	{
		mutex.lock();
		cout << "+ ";
		std::this_thread::sleep_for(1ms);
		mutex.unlock();
	}
}
void Minus()
{
	while (!finish)
	{
		mutex.lock();
		cout << "- ";
		std::this_thread::sleep_for(1ms);
		mutex.unlock();
	}
}

int main()
{
	setlocale(LC_ALL, "");

	//Plus();
	//Minus();

	std::thread plusThread(Plus);
	std::thread minusThread(Minus);

	cin.get();
	finish = true;

	if (minusThread.joinable()) minusThread.join();
	if (plusThread.joinable()) plusThread.join();

	return 0;
}