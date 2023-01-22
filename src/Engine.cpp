#include "Engine.h"
#include <GLFW/glfw3.h>
#include <vector>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
#include <list>
#include <algorithm>
Engine::Engine(GLFWwindow** WindowInput)
{
	meshCube.LoadFromObjectFile("mountain.obj");
	matProj = Matrix_MakeProjection(90.0f, coord.height / coord.width, 0.1f, 1000.0f);
	window = WindowInput;
}

void Engine::OnUpdate(float fElapsedTime, std::array<float, 3> offset, float Speed)
{

	if(glfwGetKey(*window,GLFW_KEY_UP)){
		vCamera.y += Speed * fElapsedTime;
	}
	if(glfwGetKey(*window,GLFW_KEY_DOWN)){
		vCamera.y -= Speed * fElapsedTime;
	}
	if(glfwGetKey(*window,GLFW_KEY_LEFT)){
		vCamera.x -= Speed * fElapsedTime;
	}
	if(glfwGetKey(*window,GLFW_KEY_RIGHT)){
		vCamera.x += Speed * fElapsedTime;
	}
	vec3D vForward = Vector_Mul(vLookDir, Speed * fElapsedTime);
	if(glfwGetKey(*window,GLFW_KEY_W)){
		vCamera = Vector_Add(vCamera, vForward);
	}
	if(glfwGetKey(*window,GLFW_KEY_S)){
		vCamera = Vector_Sub(vCamera, vForward);
	}
	if(glfwGetKey(*window,GLFW_KEY_A)){
		fYaw -= 2.0f * fElapsedTime;
	}
	if(glfwGetKey(*window,GLFW_KEY_D)){
		fYaw += 2.0f * fElapsedTime;
	}

	mat4x4 matRotZ, matRotX;
	// fTheta += 1.0f * fElapsedTime;
	matRotX = Matrix_MakeRotationX(0.0f);
	matRotZ = Matrix_MakeRotationZ(0.0f);

	mat4x4 matTrans;
	matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 16.0f);

	mat4x4 matWorld;
	matWorld = Matrix_Identity();
	matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
	matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

	vec3D vUp = {0,1,0};
	vec3D vTarget = {0,0,1};
	mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);
	vLookDir = Matrix_MultiplyVector(vTarget, matCameraRot);
	vTarget = Vector_Add(vCamera, vLookDir);
	mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

	mat4x4 matView = Matrix_QuickInverse(matCamera);
	std::vector<triangle> vecTrianglesToRaster;
    for(auto tri : meshCube.tris)
    {
        triangle triProjected, triTransformed, triViewed;
		
		triTransformed.p[0] = Matrix_MultiplyVector(tri.p[0], matWorld);
		triTransformed.p[1] = Matrix_MultiplyVector(tri.p[1], matWorld);
		triTransformed.p[2] = Matrix_MultiplyVector(tri.p[2], matWorld);

		vec3D normal, line1, line2;

		line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
		line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

		normal = Vector_CrossProduct(line1, line2);
		normal = Vector_Normalise(normal);
		
		vec3D vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);
		//if(normal.z < 0)
		if(Vector_DotProduct(normal, vCameraRay)< 0.0f)
		{
			vec3D lightDirection = {0.0f, 1.0f, -1.0f};
			lightDirection = Vector_Normalise(lightDirection);

			float dp = std::max(0.1f, Vector_DotProduct(lightDirection, normal));
			vec3D vOffsetView = {offset[0],offset[1],offset[2]};
			// triTransformed.p[0] = Vector_Add(triTransformed.p[0], vOffsetView);
			// triTransformed.p[1] = Vector_Add(triTransformed.p[1], vOffsetView);
			// triTransformed.p[2] = Vector_Add(triTransformed.p[2], vOffsetView);
			triTransformed.col = {dp, dp, dp, 1.0f};

			triViewed.p[0] = Matrix_MultiplyVector(triTransformed.p[0],matView);
			triViewed.p[1] = Matrix_MultiplyVector(triTransformed.p[1],matView);
			triViewed.p[2] = Matrix_MultiplyVector(triTransformed.p[2],matView);
			triViewed.col = triTransformed.col;

			int nClippedTriangles = 0;
			std::array<triangle,2> clipped;
			nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);
			
			for (int n = 0; n < nClippedTriangles; n++)
			{
				triProjected.p[0] = Matrix_MultiplyVector(clipped[n].p[0], matProj);
				triProjected.p[1] = Matrix_MultiplyVector(clipped[n].p[1], matProj);
				triProjected.p[2] = Matrix_MultiplyVector(clipped[n].p[2], matProj);
				triProjected.col = triTransformed.col;

				triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
				triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
				triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

				// triProjected.p[0].x *= -1.0f;
				// triProjected.p[1].x *= -1.0f;
				// triProjected.p[2].x *= -1.0f;
				// triProjected.p[0].y *= -1.0f;
				// triProjected.p[1].y *= -1.0f;
				// triProjected.p[2].y *= -1.0f;

				vec3D vOffsetView = {offset[0],offset[1],offset[2]};
				triTransformed.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
				triTransformed.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
				triTransformed.p[2] = Vector_Add(triProjected.p[2], vOffsetView);
				triProjected.p[0].x *= 0.5f * coord.width;
				triProjected.p[0].y *= 0.5f * coord.height;
				triProjected.p[1].x *= 0.5f * coord.width;
				triProjected.p[1].y *= 0.5f * coord.height;
				triProjected.p[2].x *= 0.5f * coord.width;
				triProjected.p[2].y *= 0.5f * coord.height;


				vecTrianglesToRaster.push_back(triProjected);
			}
		}
    }
	sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2)
	{
		float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
		float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
		return z1 > z2;
	});
	for (auto &triToRaster : vecTrianglesToRaster)
	{
		triangle clipped[2];
		std::list<triangle> listTriangles;
		listTriangles.push_back(triToRaster);
		int nNewTriangles = 1;
		for (int p = 0; p < 4; p++)
		{
			int nTrisToAdd = 0;
			while (nNewTriangles > 0)
			{
				// Take triangle from front of queue
				triangle test = listTriangles.front();
				listTriangles.pop_front();
				nNewTriangles--;

				// Clip it against a plane. We only need to test each 
				// subsequent plane, against subsequent new triangles
				// as all triangles after a plane clip are guaranteed
				// to lie on the inside of the plane. I like how this
				// comment is almost completely and utterly justified
				switch (p)
				{
					case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, coord.height - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ coord.width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				}

				// Clipping may yield a variable number of triangles, so
				// add these new ones to the back of the queue for subsequent
				// clipping against next planes
				for (int w = 0; w < nTrisToAdd; w++)
					listTriangles.push_back(clipped[w]);
			}
			nNewTriangles = listTriangles.size();

		}
		for(auto &t : listTriangles)
		{
			std::array<Pos2D, 3> Pos = {
				{{t.p[0].x, t.p[0].y},
				{t.p[1].x, t.p[1].y},
				{t.p[2].x, t.p[2].y},}
			};
			std::cout << "Render!";
			TriangleRenderer.CreateTriangle(Pos, t.col);
		}
	}
}

void Engine::OnImGUIRender()
{

}