#pragma once

class CSoft3DEngine
{
public:
	CSoft3DEngine();
	void Initilize(HWND hWnd);
	void Draw(HDC hDC);
public:
	void CreateFrameBuffer();
	inline void SetPixel(int nX, int nY, unsigned int dwColor);
	void Clear(unsigned int dwColor);
	Color RayTraceRecursive(Geometry *scene, Ray3 *ray, int maxReflect);
	void RenderScene();
private:
	HWND m_hWnd;
	int m_nWidth;
	int m_nHeight;
	int m_nStride;
	int m_nShiftX;
	int m_nShiftY;
	BYTE *m_pPixels;
	Gdiplus::Bitmap *m_pBitmap;
	PerspectiveCamera *m_camera;
	Plane *m_plane;
	Sphere *m_sphere1;
	Geometry *m_scene;
	std::vector<Light *> m_vecLightList;
};

CSoft3DEngine *CSoft3DEngine_GetInstance();