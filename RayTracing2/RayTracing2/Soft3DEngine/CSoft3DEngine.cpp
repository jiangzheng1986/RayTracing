
#include "Soft3DEngine.h"

#pragma comment(lib, "gdiplus.lib")  

CSoft3DEngine *CSoft3DEngine_GetInstance()
{
	static CSoft3DEngine s_Soft3DEngine;

	return &s_Soft3DEngine;
}

CSoft3DEngine::CSoft3DEngine()
{
	m_hWnd = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nStride = 0;
	m_nShiftX = 0;
	m_nShiftY = 0;
	m_pPixels = NULL;
	m_pBitmap = NULL;
}

void CSoft3DEngine::Initilize(HWND hWnd)
{
	m_hWnd = hWnd;

	Gdiplus::GdiplusStartupInput GdiplusStartupInput1;
	ULONG_PTR GdiToken1;
	Gdiplus::GdiplusStartup(&GdiToken1,&GdiplusStartupInput1,NULL);

	CreateFrameBuffer();

	m_camera = new PerspectiveCamera(
		Vector3(0, 5, 25),
		Vector3(0, 0, -1),
		Vector3(0, 1, 0),
		90);

	m_camera->Initialize();

	Plane *plane = new Plane(Vector3(0, 1, 0), 0);
	plane->m_material = new CheckerMaterial(0.1f, 0.5f);
	m_plane = plane;
	Sphere *sphere1 = new Sphere(Vector3(-10, 10, 0), 10);
	sphere1->m_material = new PhongMaterial(Color::s_red, Color::s_white, 16, 0.25f);
	m_sphere1 = sphere1;
	Sphere *sphere2 = new Sphere(Vector3(10, 10, 0), 10);
	sphere2->m_material = new PhongMaterial(Color::s_yellow, Color::s_white, 16, 0.25f);
	Union *scene = new Union();
	scene->AddGeometry(plane);
	scene->AddGeometry(sphere1);
	scene->AddGeometry(sphere2);

	scene->Initialize();

	m_scene = scene;

	DirectionalLight *directionalLight1 = new DirectionalLight(Color::s_white, Vector3(-1.75f, -2.0f, -1.5f));

	m_vecLightList.push_back(directionalLight1);

	PointLight *pointLight1 = new PointLight(Color::s_white.Multiply(1000), Vector3(0, 30.0f, 0));

	//m_vecLightList.push_back(pointLight1);

	SpotLight *spotLight1 = new SpotLight(Color::s_white.Multiply(2000), Vector3(0, 50.0f, 0), Vector3(0.0f, -1.0f, 0.0f),
		20.0f, 30.0f, 0.5f);

	m_vecLightList.push_back(spotLight1);

	int i;
	int nCount = m_vecLightList.size();
	for (i = 0; i < nCount; i++)
	{
		m_vecLightList[i]->Initialize();
	}

	RenderScene();
}

void CSoft3DEngine::CreateFrameBuffer()
{
	m_nWidth = BACKBUFFER_WIDTH;
	m_nHeight = BACKBUFFER_HEIGHT;
	m_nStride = m_nWidth * 4;
	m_nShiftX = 2;
	m_nShiftY = 2 + 9;

	if(m_pPixels)
	{
		delete [] m_pPixels;
		m_pPixels = NULL;
	}
	m_pPixels = new BYTE[m_nWidth * m_nHeight * 4];

	m_pBitmap = new Gdiplus::Bitmap(m_nWidth, m_nHeight, m_nStride, PixelFormat32bppARGB, m_pPixels);
}

void CSoft3DEngine::SetPixel(int nX, int nY, unsigned int dwColor)
{
	if(nX >= 0 && nY >= 0 && nX < m_nWidth && nY < m_nHeight)
	{
		*((unsigned int *)(m_pPixels + (nY << m_nShiftY) + (nX << m_nShiftX))) = dwColor;
	}
}

void CSoft3DEngine::Clear(unsigned int dwColor)
{
	int nX;
	int nY;
	for(nY = 0; nY < m_nHeight; nY++)
	{
		for(nX = 0; nX < m_nWidth; nX++)
		{
			SetPixel(nX, nY, dwColor);
		}
	}
}

Color CSoft3DEngine::RayTraceRecursive(Geometry *scene, Ray3 *ray, int maxReflect)
{
	IntersectResult result;
	scene->Intersect(ray, &result);
	if (result.m_geometry)
	{
		float reflectiveness = result.m_geometry->m_material->m_reflectiveness;
		Color color = result.m_geometry->m_material->Sample(ray, &(result.m_position), &(result.m_normal));

		LightSample lightSample;
		Color light = Color::s_black;

		int i;
		int nCount = m_vecLightList.size();
		for (i = 0; i < nCount; i++)
		{
			m_vecLightList[i]->Sample(&lightSample, m_scene, result.m_position);
			if (lightSample.m_EL.m_r > 0.0f ||
				lightSample.m_EL.m_g > 0.0f ||
				lightSample.m_EL.m_b > 0.0f)
			{
				float NdotL = result.m_normal.Dot(lightSample.m_L);
				if (NdotL > 0.0f)
				{
					light = light.Add(lightSample.m_EL.Multiply(NdotL));
				}
			}
		}
		color = color.Modulate(light);

		color = color.Multiply(1 - reflectiveness);

		if (reflectiveness > 0 && maxReflect > 0)
		{
			Vector3 r = result.m_normal.Multiply(-2.0f * result.m_normal.Dot(ray->m_direction)).Add(ray->m_direction);
			Ray3 ray1(result.m_position, r);
			Color reflectedColor = RayTraceRecursive(scene, &ray1, maxReflect - 1);
			color = color.Add(reflectedColor.Multiply(reflectiveness));
		}
		return color;
	}
	else
	{
		return Color::s_black;
	}
}

void CSoft3DEngine::RenderScene()
{
	m_sphere1->m_radius = 0.0f;
	Clear(0xFF000000);
	Ray3 ray;
	int y;
	for (y = 0; y < m_nHeight; y++)
	{
		float sy = 1 - y / (float)m_nHeight;
		int x;
		for (x = 0; x < m_nWidth; x++)
		{
			float sx = x / (float)m_nWidth;
			m_camera->GenerateRay(sx, sy, &ray);

			Color color = RayTraceRecursive(m_scene, &ray, 3);
			color.Saturate();

			unsigned char r = (unsigned char)(color.m_r * 255);
			unsigned char g = (unsigned char)(color.m_g * 255);
			unsigned char b = (unsigned char)(color.m_b * 255);
			unsigned dwColor = 0xFF000000 | b | (g << 8) | (r << 16);

			SetPixel(x, y, dwColor);
		}
	}
}

void CSoft3DEngine::Draw(HDC hDC)
{
	Gdiplus::Graphics *pGraphics1 = new Gdiplus::Graphics(hDC);

	pGraphics1->DrawImage(m_pBitmap, 0, 0);

	delete pGraphics1;

	InvalidateRect(m_hWnd, NULL, FALSE);
}