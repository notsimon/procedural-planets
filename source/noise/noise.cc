/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "noise.hh"

namespace noise {
	float fractal (Vec2f p, int octaves, float frequency, float persistence) {
		float r = 0;
		float f = frequency;
		float amplitude = 1;

		for (int i = 0; i < octaves; i++) {
			int t = i * 4096;
			Vec2f q = p * f + Vec2f(t, t);
			r += noise::simplex(q) * amplitude;
			amplitude *= persistence;
			f *= 2;
		}

		float lim = (1 - persistence) / (1 - amplitude);

		return r * lim;
	}

	float fractal (Vec3f p, int octaves, float frequency, float persistence) {
		float r = 0;
		float f = frequency;
		float amplitude = 1;

		for (int i = 0; i < octaves; i++) {
			int t = i * 4096;
			Vec3f q = p * f + Vec3f(t, t, t);
			r += noise::simplex(q) * amplitude;
			amplitude *= persistence;
			f *= 2;
		}

		float lim = (1 - persistence) / (1 - amplitude);

		return r * lim;
	}

	Vec3f grad3[] = {
		Vec3f(1,1,0), Vec3f(-1,1,0), Vec3f(1,-1,0), Vec3f(-1,-1,0),
		Vec3f(1,0,1), Vec3f(-1,0,1), Vec3f(1,0,-1), Vec3f(-1,0,-1),
		Vec3f(0,1,1), Vec3f(0,-1,1), Vec3f(0,1,-1), Vec3f(0,-1,-1)
	};

	short p[] = { 151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };
	// To remove the need for index wrapping, float the permutation table length
	short perm[512];
	short permMod12[512];

	bool init () {
		for(int i = 0; i < 512; i++) {
			perm[i]=p[i & 255];
			permMod12[i] = (short)(perm[i] % 12);
		}

		return true;
	}

	bool initialized = init();

	// Skewing and unskewing factors for 2, 3, and 4 dimensions
	float F2 = 0.5*(std::sqrt(3.0)-1.0);
	float G2 = (3.0-std::sqrt(3.0))/6.0;
	float F3 = 1.0/3.0;
	float G3 = 1.0/6.0;

	float dot(Vec3f g, float x, float y) {
		return g.x()*x + g.y()*y;
	}

	float dot(Vec3f g, float x, float y, float z) {
		return g.x()*x + g.y()*y + g.z()*z;
	}


	// 2D simplex noise
	float simplex(Vec2f p) {
		float xin = p.x();
		float yin = p.y();
		float n0, n1, n2; // Noise contributions from the three corners
		// Skew the input space to determine which simplex cell we're in
		float s = (xin+yin)*F2; // Hairy factor for 2D
		int i = std::floor(xin+s);
		int j = std::floor(yin+s);
		float t = (i+j)*G2;
		float X0 = i-t; // Unskew the cell origin back to (x,y) space
		float Y0 = j-t;
		float x0 = xin-X0; // The x,y distances from the cell origin
		float y0 = yin-Y0;
		// For the 2D case, the simplex shape is an equilateral triangle.
		// Determine which simplex we are in.
		int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
		if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
		else {i1=0; j1=1;}      // upper triangle, YX order: (0,0)->(0,1)->(1,1)
		// A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
		// a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
		// c = (3-sqrt(3))/6
		float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
		float y1 = y0 - j1 + G2;
		float x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
		float y2 = y0 - 1.0 + 2.0 * G2;
		// Work out the hashed gradient indices of the three simplex corners
		int ii = i & 255;
		int jj = j & 255;
		int gi0 = permMod12[ii+perm[jj]];
		int gi1 = permMod12[ii+i1+perm[jj+j1]];
		int gi2 = permMod12[ii+1+perm[jj+1]];
		// Calculate the contribution from the three corners
		float t0 = 0.5 - x0*x0-y0*y0;
		if(t0<0) n0 = 0.0;
		else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0);  // (x,y) of grad3 used for 2D gradient
		}
		float t1 = 0.5 - x1*x1-y1*y1;
		if(t1<0) n1 = 0.0;
		else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
		}
		float t2 = 0.5 - x2*x2-y2*y2;
		if(t2<0) n2 = 0.0;
		else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
		}
		// Add contributions from each corner to get the final noise value.
		// The result is scaled to return values in the interval [-1,1].
		return 70.0 * (n0 + n1 + n2);
	}

	// 3D simplex noise
	float simplex (Vec3f p) {
		float xin = p.x();
		float yin = p.y();
		float zin = p.z();

		float n0, n1, n2, n3; // Noise contributions from the four corners
		// Skew the input space to determine which simplex cell we're in
		float s = (xin+yin+zin)*F3; // Very nice and simple skew factor for 3D
		int i = std::floor(xin+s);
		int j = std::floor(yin+s);
		int k = std::floor(zin+s);
		float t = (i+j+k)*G3;
		float X0 = i-t; // Unskew the cell origin back to (x,y,z) space
		float Y0 = j-t;
		float Z0 = k-t;
		float x0 = xin-X0; // The x,y,z distances from the cell origin
		float y0 = yin-Y0;
		float z0 = zin-Z0;
		// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
		// Determine which simplex we are in.
		int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
		int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
		if(x0>=y0) {
			if(y0>=z0) {
				i1=1; j1=0; k1=0; i2=1; j2=1; k2=0;
			} // X Y Z order
		else if(x0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
		else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } // Z X Y order
		}
		else { // x0<y0
		if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
		else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
		else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
		}
		// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
		// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
		// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
		// c = 1/6.
		float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
		float y1 = y0 - j1 + G3;
		float z1 = z0 - k1 + G3;
		float x2 = x0 - i2 + 2.0*G3; // Offsets for third corner in (x,y,z) coords
		float y2 = y0 - j2 + 2.0*G3;
		float z2 = z0 - k2 + 2.0*G3;
		float x3 = x0 - 1.0 + 3.0*G3; // Offsets for last corner in (x,y,z) coords
		float y3 = y0 - 1.0 + 3.0*G3;
		float z3 = z0 - 1.0 + 3.0*G3;
		// Work out the hashed gradient indices of the four simplex corners
		int ii = i & 255;
		int jj = j & 255;
		int kk = k & 255;
		int gi0 = permMod12[ii+perm[jj+perm[kk]]];
		int gi1 = permMod12[ii+i1+perm[jj+j1+perm[kk+k1]]];
		int gi2 = permMod12[ii+i2+perm[jj+j2+perm[kk+k2]]];
		int gi3 = permMod12[ii+1+perm[jj+1+perm[kk+1]]];
		// Calculate the contribution from the four corners
		float t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
		if(t0<0) n0 = 0.0;
		else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
		}
		float t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
		if(t1<0) n1 = 0.0;
		else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
		}
		float t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
		if(t2<0) n2 = 0.0;
		else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
		}
		float t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
		if(t3<0) n3 = 0.0;
		else {
		t3 *= t3;
		n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
		}
		// Add contributions from each corner to get the final noise value.
		// The result is scaled to stay just inside [-1,1]
		return 32.0*(n0 + n1 + n2 + n3);
	}
}

/*float ridgedMF(double x, double z) {
	double sum = 0;
	float amplitude = 0.5f;
	float frequency = 1.0f;
	double prev = 1.0f;
	int octaves=numberOfOctaves;

	x*=hScale;
	z*=hScale;

	for (int i = 0; i < octaves; i++) {
	  double n = ridge(interpolatedNoise((float)(x*frequency),(float)(z*frequency)), offset);
	  sum += n * amplitude * prev;
	  prev = n;
	  frequency *= lacunarity;
	  amplitude *= gain;
	}

	return (float)-sum*vScale;
}

double ridge(double h, float offset) {
	h = std::abs(h);
	h = offset - h;
	h = h * h;
	return h;
}

float noise(int x, int y) {
	int n=x+y*57;
	n=(n<<13)^n;
	return (1.0-((n*(n*n*15731+789221)+1376312589)&0x7fffffff)/1073741824.0);
}

float interpolatedNoise(float x, float y) {
	int integer_X=(int)x;
	float fractional_X = x-integer_X;

	int integer_Y=(int)y;
	float fractional_Y=y-integer_Y;

	float v1 = noise(integer_X,integer_Y);
	float v2 = noise(integer_X+1,integer_Y);
	float v3 = noise(integer_X,integer_Y+1);
	float v4 = noise(integer_X+1,integer_Y+1);

	float i1=interpolate(v1,v2,fractional_X);
	float i2=interpolate(v3,v4,fractional_X);

	return interpolate(i1,i2,fractional_Y);
}*/