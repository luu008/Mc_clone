#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <queue>
class truck
{
	unsigned long long checkError();

private:
	std::queue<unsigned long long> errorList;
};

 unsigned long long truck::checkError()
{
	 if (errorList.empty())
		 return 0;
	 unsigned long long error = errorList.front();
	 errorList.pop();
	 return error;
}