#include "RayTracing.h"
Vector3::Vector3()
{
	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 0.0f;
}

Vector3::Vector3(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}

Vector3::~Vector3()
{

}

Vector3 Vector3::Copy()
{
	return Vector3(m_x, m_y, m_z);
}

float Vector3::Length()
{
	return sqrtf(m_x * m_x + m_y * m_y + m_z * m_z);
}

float Vector3::SqrLength()
{
	return m_x * m_x + m_y * m_y + m_z * m_z;
}

Vector3 Vector3::Normalize()
{
	float inv = 1.0f / Length();
	return Vector3(m_x * inv, m_y * inv, m_z * inv);
}

Vector3 Vector3::Negate()
{
	return Vector3(-m_x, -m_y, -m_z);
}

Vector3 Vector3::Add(const Vector3 &v)
{
	return Vector3(m_x + v.m_x, m_y + v.m_y, m_z + v.m_z);
}

Vector3 Vector3::Subtract(const Vector3 &v)
{
	return Vector3(m_x - v.m_x, m_y - v.m_y, m_z - v.m_z);
}

Vector3 Vector3::Multiply(float f)
{
	return Vector3(m_x * f, m_y * f, m_z * f);
}

Vector3 Vector3::Divide(float f)
{
	float inv = 1.0f / f;
	return Vector3(m_x * inv, m_y * inv, m_z * inv);
}

float Vector3::Dot(const Vector3 &v)
{
	return m_x * v.m_x + m_y * v.m_y + m_z * v.m_z;
}

Vector3 Vector3::Cross(const Vector3 &v)
{
	return Vector3(-m_z * v.m_y + m_y * v.m_z,
		m_z * v.m_x - m_x * v.m_z,
		-m_y * v.m_x + m_x * v.m_y);
}

Vector3 Vector3::s_zero = Vector3(0, 0, 0);

Ray3::Ray3()
{
	m_origin = Vector3(0, 0, 0);
	m_direction = Vector3(1, 0, 0);
}

Ray3::Ray3(const Vector3 &origin, const Vector3 &direction)
{
	m_origin = origin;
	m_direction = direction;
}

Ray3::~Ray3()
{

}

Vector3 Ray3::GetPoint(float t)
{
	return m_origin.Add(m_direction.Multiply(t));
}

IntersectResult::IntersectResult()
{
	m_geometry = NULL;
	m_distance = 0;
}

IntersectResult::~IntersectResult()
{

}

IntersectResult IntersectResult::s_noHit = IntersectResult();

Geometry::Geometry()
{
	m_material = NULL;
}

Sphere::Sphere(Vector3 center, float radius)
{
	m_center = center;
	m_radius = radius;
	m_sqrRadius = 0.0f;
}

Sphere::~Sphere()
{
	
}

void Sphere::Initialize()
{
	m_sqrRadius = m_radius * m_radius;
}

void Sphere::Intersect(Ray3 *ray, IntersectResult *intersectResult)
{
	Vector3 v = ray->m_origin.Subtract(m_center);

	float a0 = v.SqrLength() - m_sqrRadius;

	float DdotV = ray->m_direction.Dot(v);

	if (DdotV <= 0.0f)
	{
		float discr = DdotV * DdotV - a0;
		if (discr >= 0.0f)
		{
			intersectResult->m_geometry = this;
			intersectResult->m_distance = -DdotV - sqrtf(discr);
			intersectResult->m_position = ray->GetPoint(intersectResult->m_distance);
			intersectResult->m_normal = intersectResult->m_position.Subtract(m_center).Normalize();
			return;
		}
	}

	*intersectResult = IntersectResult::s_noHit;

	return;
}

Plane::Plane(Vector3 normal, float d)
{
	m_normal = normal;

	m_d = d;
}

Plane::~Plane() 
{
}

void Plane::Initialize() 
{
	m_position = m_normal.Multiply(m_d);
}

void Plane::Intersect(Ray3 *ray, IntersectResult *intersectResult) 
{
	float a = ray->m_direction.Dot(m_normal);

	if (a >= 0)
	{
		*intersectResult = IntersectResult::s_noHit;
		return;
	}

	float b = m_normal.Dot(ray->m_origin.Subtract(m_position));

	intersectResult->m_geometry = this;
	intersectResult->m_distance = -b / a;
	intersectResult->m_position = ray->GetPoint(intersectResult->m_distance);
	intersectResult->m_normal = m_normal;
}

Union::Union() 
{

}

Union::~Union() 
{

}

void Union::AddGeometry(Geometry *geometry) 
{
	m_geometies.push_back(geometry);
}

void Union::Initialize()
{
	int nCount = m_geometies.size();
	int i;
	for (i = 0; i < nCount; i++)
	{
		m_geometies[i]->Initialize();
	}
}

void Union::Intersect(Ray3 *ray, IntersectResult *intersectResult) 
{
	float minDistance = 10000.0f;
	IntersectResult minResult = IntersectResult::s_noHit;

	int nCount = m_geometies.size();
	int i;
	for (i = 0; i < nCount; i++)
	{
		IntersectResult result;
		m_geometies[i]->Intersect(ray, &result);

		if (result.m_geometry && result.m_distance < minDistance)
		{
			minDistance = result.m_distance;
			minResult = result;
		}
	}

	*intersectResult = minResult;
}

PerspectiveCamera::PerspectiveCamera(const Vector3 &eye, const Vector3 &front, const Vector3 &up, float fov)
{
	m_eye = eye;
	m_front = front;
	m_refUp = up;
	m_fov = fov;
}

PerspectiveCamera::~PerspectiveCamera()
{

}

void PerspectiveCamera::Initialize()
{
	m_right = m_front.Cross(m_refUp);
	m_up = m_right.Cross(m_front);
	m_fovScale = tanf(m_fov * 0.5f * M_PI_F / 180) * 2;
}

void PerspectiveCamera::GenerateRay(float x, float y, Ray3 *ray)
{
	Vector3 r = m_right.Multiply((x - 0.5f) * m_fovScale);
	Vector3 u = m_up.Multiply((y - 0.5f) * m_fovScale);
	ray->m_origin = m_eye;
	ray->m_direction = m_front.Add(r).Add(u).Normalize();
}

Color::Color()
{
	m_r = 0.0f;
	m_g = 0.0f;
	m_b = 0.0f;
}

Color::Color(float r, float g, float b)
{
	m_r = r;
	m_g = g;
	m_b = b;
}

Color::~Color()
{

}

Color Color::Add(const Color &c)
{
	return Color(m_r + c.m_r, m_g + c.m_g, m_b + c.m_b);
}

Color Color::Multiply(float s)
{
	return Color(m_r * s, m_g * s, m_b * s);
}

Color Color::Modulate(const Color &c)
{
	return Color(m_r * c.m_r, m_g * c.m_g, m_b * c.m_b);
}

void Color::Saturate()
{
	m_r = MIN_(m_r, 1.0f);
	m_g = MIN_(m_g, 1.0f);
	m_b = MIN_(m_b, 1.0f);
}

Color Color::s_black = Color(0.0f, 0.0f, 0.0f);
Color Color::s_white = Color(1.0f, 1.0f, 1.0f);
Color Color::s_red = Color(1.0f, 0.0f, 0.0f);
Color Color::s_green = Color(0.0f, 1.0f, 0.0f);
Color Color::s_blue = Color(0.0f, 0.0f, 1.0f);
Color Color::s_yellow = Color(1.0f, 1.0f, 0.0f);

CheckerMaterial::CheckerMaterial(float scale, float reflectiveness)
{
	m_scale = scale;
	m_reflectiveness = reflectiveness;
}

Color CheckerMaterial::Sample(Ray3 *ray, Vector3 *position, Vector3 *normal)
{
	bool b1 = ((int)((position->m_x + 1000.0001f) * m_scale)) % 2 == 0;
	bool b2 = ((int)((position->m_z + 1000.0001f) * m_scale)) % 2 == 0;
	return (b1 ^ b2) ? Color::s_black : Color::s_white;
}

PhongMaterial::PhongMaterial(const Color &diffuse, const Color &specular,
	float shininess, float reflectiveness)
	: m_diffuse(diffuse)
	, m_specular(specular)
{
	m_shininess = shininess;
	m_reflectiveness = reflectiveness;
}

PhongMaterial::~PhongMaterial()
{

}

Color PhongMaterial::Sample(Ray3 *ray, Vector3 *position, Vector3 *normal)
{
	return m_diffuse;
}

LightSample::LightSample()
	: m_L(Vector3::s_zero)
	, m_EL(Color::s_black)
{
	
}

LightSample::LightSample(const Vector3 &L, const Color &EL)
{
	m_L = L;
	m_EL = EL;
}

LightSample::~LightSample()
{

}
 
LightSample LightSample::s_zero = LightSample(Vector3::s_zero, Color::s_black);

Light::Light()
{
	m_shadow = true;
}

DirectionalLight::DirectionalLight(const Color &irradiance, const Vector3 &direction)
{
	m_irradiance = irradiance;
	m_direction = direction;
}

DirectionalLight::~DirectionalLight()
{

}

void DirectionalLight::Initialize()
{
	m_L = m_direction.Normalize().Negate();
}

void DirectionalLight::Sample(LightSample *lightSample, Geometry *scene, const Vector3 &position)
{
	if (m_shadow)
	{
		Ray3 shadowRay(position, m_L);
		IntersectResult shadowResult;
		scene->Intersect(&shadowRay, &shadowResult);
		if (shadowResult.m_geometry)
		{
			*lightSample = LightSample::s_zero;
			return;
		}
	}

	lightSample->m_L = m_L;
	lightSample->m_EL = m_irradiance;
}

PointLight::PointLight(const Color &intensity, const Vector3 &position)
{
	m_intensity = intensity;
	m_position = position;
}

PointLight::~PointLight()
{

}

void PointLight::Initialize()
{

}

void PointLight::Sample(LightSample *lightSample, Geometry *scene, const Vector3 &position)
{
	Vector3 delta = m_position.Subtract(position);
	float rr = delta.SqrLength();
	float r = sqrtf(rr);
	Vector3 L = delta.Divide(r);

	float attenuation = 1.0f / rr;

	Color EL = m_intensity.Multiply(attenuation);

	if (EL.m_r < EPSILON_VALUE_1 &&
		EL.m_g < EPSILON_VALUE_1 &&
		EL.m_b < EPSILON_VALUE_1)
	{
		*lightSample = LightSample::s_zero;
		return;
	}

	if (m_shadow)
	{
		Ray3 shadowRay(position, L);
		IntersectResult shadowResult;
		scene->Intersect(&shadowRay, &shadowResult);
		if (shadowResult.m_geometry)
		{
			*lightSample = LightSample::s_zero;
			return;
		}
	}

	lightSample->m_L = L;
	lightSample->m_EL = EL;
}

SpotLight::SpotLight(const Color &intensity, const Vector3 &position, const Vector3 &direction,
	float theta, float phi, float falloff)
{
	m_intensity = intensity;
	m_position = position;
	m_direction = direction;
	m_theta = theta;
	m_phi = phi;
	m_falloff = falloff;
}

SpotLight::~SpotLight()
{

}

void SpotLight::Initialize()
{
	m_S = m_direction.Normalize().Negate();
	m_cosTheta = cosf(m_theta * M_PI_F / 180 / 2);
	m_cosPhi = cosf(m_phi * M_PI_F / 180 / 2);
	m_baseMultiplier = 1.0f / (m_cosTheta - m_cosPhi);
}

void SpotLight::Sample(LightSample *lightSample, Geometry *scene, const Vector3 &position)
{
	Vector3 delta = m_position.Subtract(position);
	float rr = delta.SqrLength();
	float r = sqrtf(rr);
	Vector3 L = delta.Divide(r);

	float spot = 0.0f;
	float SdotL = m_S.Dot(L);
	if (SdotL >= m_cosTheta)
	{
		spot = 1.0f;
	}
	else if (SdotL <= m_cosPhi)
	{
		spot = 0.0f;

		*lightSample = LightSample::s_zero;
		return;
	}
	else
	{
		spot = powf((SdotL - m_cosPhi) * m_baseMultiplier, m_falloff);
	}

	float attenuation = 1.0f / rr;

	Color EL = m_intensity.Multiply(attenuation * spot);

	if (EL.m_r < EPSILON_VALUE_1 &&
		EL.m_g < EPSILON_VALUE_1 &&
		EL.m_b < EPSILON_VALUE_1)
	{
		*lightSample = LightSample::s_zero;
		return;
	}

	if (m_shadow)
	{
		Ray3 shadowRay(position, L);
		IntersectResult shadowResult;
		scene->Intersect(&shadowRay, &shadowResult);
		if (shadowResult.m_geometry)
		{
			*lightSample = LightSample::s_zero;
			return;
		}
	}

	lightSample->m_L = L;
	lightSample->m_EL = EL;
}