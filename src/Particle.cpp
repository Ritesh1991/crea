#include "Particle.h"

Particle::Particle(){
    isAlive         = true;
    isTouched		= false;
    immortal        = false;
    bounces         = false;
    sizeAge         = false;
    opacityAge      = false;
    colorAge        = false;
    flickersAge     = false;
    isEmpty         = false;
    drawLine        = false;
    age             = 0;
    width           = ofGetWidth();
    height          = ofGetHeight();
}

void Particle::setup(float id, ofPoint pos, ofPoint vel, ofColor color, float initialRadius, float lifetime){
    this->id = id;
    this->pos = pos;
    this->vel = vel;
    this->color = color;
    this->initialRadius = initialRadius;
    this->lifetime = lifetime;

    this->radius = initialRadius;
    this->mass = initialRadius * initialRadius * 0.005f;
    this->prevPos = pos;
    this->iniPos = pos;
    this->originalHue = color.getHue();
}

void Particle::update(float dt){
    if(isAlive){

        // Update position
        acc += frc;
        vel += acc;
        pos += vel*dt;
        vel *= friction;
        acc.set(0, 0);
        frc.set(0, 0);

        // Update age and check if particle has to die
        if(!immortal){
        	age += dt;
	        if(age >= lifetime) isAlive = false;
        }

        // Decrease particle radius with age
        if (sizeAge) radius = initialRadius * (1.0f - (age/lifetime));

        // Decrease particle opacity with age
        opacity = 255;
        if (opacityAge) opacity *= (1.0f - (age/lifetime));
        if (flickersAge && (age/lifetime) > 0.94f && ofRandomf() > 0.3) opacity *= 0.2;

        // Change particle color with age
        if (colorAge){
            float saturation = ofMap(age, 0, lifetime, 255, 128);
            float hue = ofMap(age, 0, lifetime, originalHue, originalHue-100);
            color.setSaturation(saturation);
            color.setHue(hue);
        }

        // Bounce particle with the window margins
        if(bounces){
        	bool isBouncing = false;

            if(pos.x > width-radius){
                pos.x = width-radius;
                vel.x *= -1.0;
            }
            else if(pos.x < radius){
                pos.x = radius;
                vel.x *= -1.0;
            }

            if(pos.y > height-radius){
                pos.y = height-radius;
                vel.y *= -1.0;
                isBouncing = true;
            }
            else if(pos.y < radius){
                pos.y = radius;
                vel.y *= -1.0;
                isBouncing = true;
            }

            if (isBouncing){
	            vel *= 0.9;
//	            vel.y *= -0.5;
            }
        }
    }
}

void Particle::draw(){
    if(isAlive){
        ofPushStyle();

        ofSetColor(color, opacity);

        if(isEmpty){
            ofNoFill();
            ofSetLineWidth(1);
        }
        else{
            ofFill();
        }

        if(!drawLine){
            int resolution = ofMap(radius, 0, 10, 6, 22, true);
            ofSetCircleResolution(resolution);
            ofCircle(pos, radius);
        }
        else{
            ofLine(pos, prevPos);
            prevPos = pos;
        }

        // // Draw arrows
        // if (markerDist == 0){
        //     ofCircle(pos, 2);
        // }
        // else{
        //     float length = 15.0f;
        //     ofPoint p1(pos);
        //     ofPoint p2(pos + dir*length);
        //     ofLine(p1, p2);
        // }

        ofPopStyle();
    }
}

void Particle::addForce(ofPoint force){
	frc += force/mass;
}

void Particle::addNoise(float angle, float turbulence, float dt){
    // Perlin noise
	float noise = ofNoise(pos.x * 0.005f,  pos.y * 0.005f, dt * 0.1f) * angle;
    ofPoint noiseVector(cos(noise), sin(noise));
    frc += noiseVector * turbulence * age * 0.1;
}

void Particle::addRepulsionForce(Particle &p, float radiusSqrd, float scale){

	// ----------- (1) make a vector of where this particle p is:
	ofPoint posOfForce;
	posOfForce.set(p.pos.x, p.pos.y);

	// ----------- (2) calculate the difference & length
	ofVec2f dir         = pos - posOfForce;
	float lengthSqrd    = pos.squareDistance(posOfForce); // faster than length or distance (no square root)

	// ----------- (3) check close enough
	bool closeEnough = true;
	if (radiusSqrd > 0){
	    if (lengthSqrd > radiusSqrd){
	        closeEnough = false;
	    }
	}

	// ----------- (4) if so, update force
	if (closeEnough == true){
	    float pct = 1 - (lengthSqrd / radiusSqrd);  // stronger on the inside
	    dir.normalize();
	    frc   += dir * scale * pct;
	    p.frc -= dir * scale * pct;
	}
}

void Particle::addRepulsionForce(Particle &p, float scale){

	// ----------- (1) make a vector of where this particle p is:
	ofPoint posOfForce;
	posOfForce.set(p.pos.x, p.pos.y);

	// ----------- (2) calculate the difference & length
	ofVec2f dir         = pos - posOfForce;
	float lengthSqrd    = pos.squareDistance(posOfForce); // faster than length or distance (no square root)
	float radius        = this->radius + p.radius;
	float radiusSqrd    = radius*radius;

	// ----------- (3) check close enough
	bool closeEnough = true;
	if (radiusSqrd > 0){
	    if (lengthSqrd > radiusSqrd){
	        closeEnough = false;
	    }
	}

	// ----------- (4) if so, update force
	if (closeEnough == true){
	    float pct = 1 - (lengthSqrd / radiusSqrd);  // stronger on the inside
	    dir.normalize();
	    frc   += dir * scale * pct;
	    p.frc -= dir * scale * pct;
	}
}

void Particle::addRepulsionForce(float x, float y, float radiusSqrd, float scale){

    // ----------- (1) make a vector of where this position is:
	ofPoint posOfForce;
	posOfForce.set(x, y);

    // ----------- (2) calculate the difference & length
	ofPoint dir	        = pos - posOfForce;
	float lengthSqrd	= pos.squareDistance(posOfForce); // faster than length or distance (no square root)

    // ----------- (3) check close enough
    bool closeEnough = true;
    if (radiusSqrd > 0){
        if (lengthSqrd > radiusSqrd){
            closeEnough = false;
        }
    }

    // ----------- (4) if so, update force
    if (closeEnough == true){
		float pct = 1 - (lengthSqrd / radiusSqrd);  // stronger on the inside
        dir.normalize();
        frc += dir * scale * pct;
    }
}

void Particle::addAttractionForce(Particle &p, float radiusSqrd, float scale){

	// ----------- (1) make a vector of where this particle p is:
	ofPoint posOfForce;
	posOfForce.set(p.pos.x, p.pos.y);

	// ----------- (2) calculate the difference & length
	ofVec2f dir         = pos - posOfForce;
	float lengthSqrd    = pos.squareDistance(posOfForce); // faster than length or distance (no square root)

	// ----------- (3) check close enough
	bool closeEnough = true;
	if (radiusSqrd > 0){
	    if (lengthSqrd > radiusSqrd){
	        closeEnough = false;
	    }
	}

	// ----------- (4) if so, update force
	if (closeEnough == true){
	    float pct = 1 - (lengthSqrd / radiusSqrd);  // stronger on the inside
	    dir.normalize();
        frc   -= dir * scale * pct;
	    p.frc += dir * scale * pct;
	}
}

void Particle::addAttractionForce(float x, float y, float radiusSqrd, float scale){

    // ----------- (1) make a vector of where this position is:
	ofPoint posOfForce;
	posOfForce.set(x, y);

    // ----------- (2) calculate the difference & length

	ofPoint dir	        = pos - posOfForce;
	float lengthSqrd	= pos.squareDistance(posOfForce); // faster than length or distance (no square root)

    // ----------- (3) check close enough

    bool closeEnough = true;
    if (radiusSqrd > 0){
        if (lengthSqrd > radiusSqrd){
            closeEnough = false;
        }
    }

    // ----------- (4) if so, update force
    if (closeEnough == true){
		float pct = 1 - (lengthSqrd / radiusSqrd);  // stronger on the inside
        dir.normalize();
        frc -= dir * scale * pct;
    }
}

//------------------------------------------------------------------
void Particle::xenoToOrigin(float spd){

    pos.x = spd * iniPos.x + (1-spd) * pos.x;
    pos.y = spd * iniPos.y + (1-spd) * pos.y;

    // pos.x = spd * catchX + (1-spd) * pos.x; - Zachs equation
    // xeno math explianed
    // A------B--------------------C
    // A is beginning, C is end
    // say you wanna move .25 of the remaining dist each iteration
    // your first iteration you moved to B, wich is 0.25 of the distance between A and C
    // the next iteration you will move .25 the distance between B and C
    // let the next iteration be called 'new'
    // pos.new = pos.b + (pos.c-pos.b)*0.25
    // now let's simplify this equation
    // pos.new = pos.b(1-.25) + pos.c(.25)
    // since pos.new and pos.b are analogous to pos.x
    // and pos.c is analogous to catchX
    // we can write pos.x = pos.x(1-.25) + catchX(.25)
    // this equation is the same as Zachs simplified equation

}

void Particle::kill(){
    isAlive = false;
}

