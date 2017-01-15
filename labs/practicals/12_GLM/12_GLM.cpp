#include <glm\glm.hpp>
#include <glm\gtc\constants.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\euler_angles.hpp>
#include <glm\gtx\projection.hpp>
#include <iostream>

using namespace std;
using namespace glm;

// translation matrix

vec3 v1(1.0f, 1.0f, 0.0f);
mat4 t1 = translate(mat4(1.0f), v1);

// quaternions

quat q;




int main() {}