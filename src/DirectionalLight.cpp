#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(const std::string &lightName, const sf::Vector2f &initialPosition, const sf::Color &color, const float initailItensity, const float angleIn, const float openAngle)
    : Light(), lightKey(lightName), lightVector(initialPosition), lightColor(color), intensity(initailItensity), facingAngle(angleIn), openingAngle(openAngle)
{
    buildLightRays(directionalRays);
}

void DirectionalLight::buildLightRays(std::vector<sf::Vector2f> &lightRays)
{

    float fangle     = facingAngle * M_PI / 180;
    float offsetAngle   = openingAngle * M_PI / 180;
    //float radius = 200; TODO use a radius value

    lightRays.clear();
    float endAngle = fangle+offsetAngle/2;
    for(float angle=(fangle-offsetAngle/2); angle< endAngle; angle+=(M_PI*2)/100)
    {
        directionalRays.push_back(sf::Vector2f(lightVector.x + cos(angle+0.01f) ,lightVector.y + sin(angle)));
        directionalRays.push_back(sf::Vector2f(lightVector.x + cos(angle) ,lightVector.y + sin(angle)));
        directionalRays.push_back(sf::Vector2f(lightVector.x + cos(angle-0.01f) ,lightVector.y + sin(angle)));

    }
}


DirectionalLight::~DirectionalLight()
{
}


sf::Vector2f DirectionalLight::getVec()
{
    return lightVector;
}


std::string DirectionalLight::getKey()
{
    return lightKey;
}

void DirectionalLight::setVec(const sf::Vector2f &lightVec)
{
    lightVector = lightVec;
    buildLightRays(directionalRays);
}

sf::Color DirectionalLight::getColor()
{
    return lightColor;
}

float DirectionalLight::getIntensity()
{
    return intensity;
}

bool DirectionalLight::compareIntersects(Intersect vec1, Intersect vec2)
{
    if(vec1.getAngle()  - vec2.getAngle() < 0)
    {
        return true;
    }
    else if (vec1.getAngle()  - vec2.getAngle() > 0)
    {
        return false;
    }

    return false;
}

std::vector<Intersect> DirectionalLight::getIntersectPoints( std::vector<sf::Vector2f> &shapeVectors, const std::vector<float> &uniqueAngles)
{

    std::vector<Intersect> intersects;
//    for(int uniqueAngleIndex=0; uniqueAngleIndex < directionalRays.size(); uniqueAngleIndex++)
//    {

    float fangle     = facingAngle * M_PI / 180;
    float offsetAngle   = openingAngle * M_PI / 180;
    //float radius = 200; TODO use a radius value

    float endAngle = fangle+offsetAngle/2;
    float startAngle = fangle-offsetAngle/2;

    for(int uniqueAngleIndex=0; uniqueAngleIndex < uniqueAngles.size(); uniqueAngleIndex++)
    {
        float angle = uniqueAngles[uniqueAngleIndex];

        float x = cos(angle);
        float y = sin(angle);

        sf::Vector2f rayline(x + lightVector.x, y +lightVector.y );
        sf::VertexArray ray(sf::Lines);

        ray.append(sf::Vertex(lightVector, sf::Color::Black));
        ray.append(sf::Vertex(rayline, sf::Color::Black));

        Intersect closestInterect = getIntersect(shapeVectors, ray);

        if(closestInterect.getParam() < 1000)
        {
            float a = atan2( closestInterect.getIntersectPoint().y -lightVector.y , closestInterect.getIntersectPoint().x -lightVector.x  );

            if(a > startAngle && a < endAngle)
            {
                intersects.push_back(closestInterect);
            }
        }


    }

    sf::VertexArray ray(sf::Lines);
    ray.append(sf::Vertex(lightVector, sf::Color::Black));
    ray.append(sf::Vertex(directionalRays[0], sf::Color::Black));

    Intersect closestInterect = getIntersect(shapeVectors, ray);

    if(closestInterect.getParam() < 1000)
    {
        intersects.push_back(closestInterect);
    }

    sf::VertexArray ray2(sf::Lines);
    ray2.append(sf::Vertex(lightVector, sf::Color::Black));
    ray2.append(sf::Vertex(directionalRays[directionalRays.size()-1], sf::Color::Black));

    Intersect closestInterect2 = getIntersect(shapeVectors, ray2);

    if(closestInterect2.getParam() < 1000)
    {
        intersects.push_back(closestInterect2);
    }


    std::sort(intersects.begin(), intersects.end(), compareIntersects);
    return intersects;
}

Intersect DirectionalLight::getIntersect(std::vector<sf::Vector2f> &shapeVectors,  sf::VertexArray ray)
{
    Intersect closestInterect(sf::Vector2f(799,799), 1000);
    for(int i = 0 ;  i < shapeVectors.size(); i+=2)
    {
        sf::Vector2f seg1 =shapeVectors[i];
        sf::Vector2f seg2 =shapeVectors[i+1];
        sf::VertexArray segLine(sf::Lines);
        segLine.append(seg1);
        segLine.append(seg2);
        Intersect intersect =  VectorMath::getLineIntersect(ray, segLine);

        if(intersect.getIntersectPoint().x > 0 && intersect.getIntersectPoint().y > 0)
        {
            //add only the intersect with the smallest magnitude since it will be closest intersect
            if(intersect.getParam() < closestInterect.getParam())
            {
                closestInterect = intersect;
            }
        }

    }
    return closestInterect;
}



void DirectionalLight::generateLight(std::vector<sf::Vector2f> &shapePoints, std::vector<float> &uniqueAngles)
{

    std::vector<Intersect> intersects = getIntersectPoints(shapePoints,uniqueAngles);


    sf::VertexArray rayLine(sf::TrianglesFan);
    rayLine.append(sf::Vertex(lightVector, sf::Color::White));

    sf::VertexArray rays(sf::Lines);

    for(int i = 0; i < intersects.size(); i++)
    {
        rayLine.append(sf::Vertex(intersects[i].getIntersectPoint(), sf::Color::White));

        if(shouldDebugLines)
        {
            rays.append(sf::Vertex(lightVector, sf::Color::Red));
            rays.append(sf::Vertex(intersects[i].getIntersectPoint(), sf::Color::Red));
        }
    }

     rayLine.append(sf::Vertex(intersects[0].getIntersectPoint(),  sf::Color::White));

    lightVertexArray = rayLine;
    if(shouldDebugLines)
    {
        debugRays = rays;
    }

}


void DirectionalLight::render(sf::RenderTarget &target, sf::RenderStates &renderState)
{

    target.draw(lightVertexArray, renderState);

    if(shouldDebugLines)
    {
        target.draw(debugRays);
    }

}