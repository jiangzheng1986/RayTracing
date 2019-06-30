
#pragma once

class Vector3
{
public:
	Vector3();
	Vector3(float x, float y, float z);
	~Vector3();
	Vector3 Copy();
	float Length();
	float SqrLength();
	Vector3 Normalize();
	Vector3 Negate();
	Vector3 Add(const Vector3 &v);
	Vector3 Subtract(const Vector3 &v);
	Vector3 Multiply(float f);
	Vector3 Divide(float f);
	float Dot(const Vector3 &v);
	Vector3 Cross(const Vector3 &v);
	static Vector3 s_zero;
public:
	float m_x;
	float m_y;
	float m_z;
};

class Ray3
{
public:
	Ray3();
	Ray3(const Vector3 &origin, const Vector3 &direction);
	~Ray3();
	Vector3 GetPoint(float t);
public:
	Vector3 m_origin;
	Vector3 m_direction;
};

class Geometry;

class IntersectResult
{
public:
	IntersectResult();
	~IntersectResult();
public:
	Geometry *m_geometry;
	float m_distance;
	Vector3 m_position;
	Vector3 m_normal;
public:
	static IntersectResult s_noHit;
};

class Material;

class Geometry
{
public:
	Geometry();
	virtual void Initialize() = 0;
	virtual void Intersect(Ray3 *ray, IntersectResult *intersectResult) = 0;
public:
	Material *m_material;
};

class Sphere : public Geometry
{
public:
	Sphere(Vector3 center, float radius);
	~Sphere();
	void Initialize() override;
	void Intersect(Ray3 *ray, IntersectResult *intersectResult) override;
public:
	Vector3 m_center;
	float m_radius;
private:
	float m_sqrRadius;
};

class Plane : public Geometry
{
public:
	Plane(Vector3 normal, float d);
	~Plane();
	void Initialize() override;
	void Intersect(Ray3 *ray, IntersectResult *intersectResult) override;
public:
	Vector3 m_normal;
	float m_d;
private:
	Vector3 m_position;
};

class Union : public Geometry
{
public:
	Union();
	~Union();
	void AddGeometry(Geometry *geometry);
	void Initialize() override;
	void Intersect(Ray3 *ray, IntersectResult *intersectResult) override;
public:
	std::vector<Geometry *> m_geometies;
};

class PerspectiveCamera
{
public:
	PerspectiveCamera(const Vector3 &eye, const Vector3 &front, const Vector3 &up, float fov);
	~PerspectiveCamera();
	void Initialize();
	void GenerateRay(float x, float y, Ray3 *ray);
public:
	Vector3 m_eye;
	Vector3 m_front;
	Vector3 m_refUp;
	float m_fov;
public:
	Vector3 m_right;
	Vector3 m_up;
	float m_fovScale;
};

class Color
{
public:
	Color();
	Color(float r, float g, float b);
	~Color();
	Color Add(const Color &c);
	Color Multiply(float s);
	Color Modulate(const Color &c);
	void Saturate();
public:
	float m_r;
	float m_g;
	float m_b;
public:
	static Color s_black;
	static Color s_white;
	static Color s_red;
	static Color s_green;
	static Color s_blue;
	static Color s_yellow;
};

class Material
{
public:
	virtual Color Sample(Ray3 *ray, Vector3 *position, Vector3 *normal) = 0;
public:
	float m_reflectiveness;
};

class CheckerMaterial : public Material
{
public:
	CheckerMaterial(float scale, float reflectiveness);
	Color Sample(Ray3 *ray, Vector3 *position, Vector3 *normal) override;
public:
	float m_scale;
};

class PhongMaterial : public Material
{
public:
	PhongMaterial(const Color &diffuse, const Color &specular,
		float shininess, float reflectiveness);
	~PhongMaterial();
	Color Sample(Ray3 *ray, Vector3 *position, Vector3 *normal) override;
public:
	Color m_diffuse;
	Color m_specular;
	float m_shininess;
};

class LightSample
{
public:
	LightSample();
	LightSample(const Vector3 &L, const Color &EL);
	~LightSample();
	static LightSample s_zero;
public:
	Vector3 m_L;
	Color m_EL;
};

class Light
{
public:
	Light();
	virtual void Initialize() = 0;
	virtual void Sample(LightSample *lightSample, Geometry *scene, const Vector3 &position) = 0;
	bool m_shadow;
};

class DirectionalLight : public Light
{
public:
	DirectionalLight(const Color &irradiance, const Vector3 &direction);
	~DirectionalLight();
	void Initialize() override;
	void Sample(LightSample *lightSample, Geometry *scene, const Vector3 &position) override;
private:
	Color m_irradiance;
	Vector3 m_direction;
	Vector3 m_L;
};

class PointLight : public Light
{
public:
	PointLight(const Color &intensity, const Vector3 &position);
	~PointLight();
	void Initialize() override;
	void Sample(LightSample *lightSample, Geometry *scene, const Vector3 &position) override;
private:
	Color m_intensity;
	Vector3 m_position;
};

class SpotLight : public Light
{
public:
	SpotLight(const Color &intensity, const Vector3 &position, const Vector3 &direction, 
		float theta, float phi, float falloff);
	~SpotLight();
	void Initialize() override;
	void Sample(LightSample *lightSample, Geometry *scene, const Vector3 &position) override;
private:
	Color m_intensity;
	Vector3 m_position;
	Vector3 m_direction;
	float m_theta;
	float m_phi;
	float m_falloff;
private:
	Vector3 m_S;
	float m_cosTheta;
	float m_cosPhi;
	float m_baseMultiplier;
};