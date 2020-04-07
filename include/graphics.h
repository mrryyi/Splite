#pragma once
#include "pre.h"
#include "game.h"
#include "shaders.h"
// Our fancy schmancy graphics handler
#include <GLFW/glfw3.h>
#include <cmath>
#include <fstream>
#include <strstream>


namespace graphics
{


class Rect {
public:
    float32 x_start = 0.0;
    float32 y_start = 0.0;
    float32 x_end = 0.0;
    float32 y_end = 0.0;
    Rect(const float32 x1, const float32 y1, const float32 x2, const float32 y2)
    {
        x_start = x1;
        y_start = y1;
        x_end = x2;
        y_end = y2;
    };

    Rect() {};

    void scale(const float32 scale) {
        x_end *= scale;
        y_end *= scale;
    };

    void render( bool8 state_got ) {
    };
    
};


class Rect_w : public Rect {
public:
    float32 width = 1.0;
    float32 height = 1.0;

    Rect_w(){};
    Rect_w(const float32 x, const float32 y, const float32 width, const float32 height) {
        this->width = width;
        this->height = height;
        x_start = x;
        y_start = y;
        x_end = x_start + width;
        y_end = y_start + height;
    };
};







struct vec3d {
    float32 x = 0.0;
    float32 y = 0.0;
    float32 z = 0.0;
    float32 w = 1.0;
};

struct color3f {
    GLfloat r = { 1.0 }, g = { 1.0 }, b = { 1.0 };

    void operator=(const color3f& param) {
        r = param.r;
        g = param.g;
        b = param.b;
    }
};

struct triangle {
    vec3d p[3];

    color3f color;


};

struct mesh {
    std::vector<triangle> tris;

    bool8 load_from_object_file( std::string filename ) {
        
        std::ifstream f(filename);
        if (!f.is_open())
            return false;
        
        std::vector<vec3d> verts;
        char junk;

        while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			std::strstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}

        return true;
    }
};

struct mat4x4 {
    float32 m[4][4] = { 0 };
};










class GraphicsHandle {
private:
    mesh meshCube;
    mat4x4 matProj;

    int32 framebuffer_height = 0;
    int32 framebuffer_width = 0;

    // Placeholder for camera.
    vec3d vCamera;
    vec3d vLookDir;
    float32 fYaw = 0.0;

    float32 fTheta = 0.0;

    vec3d matrix_multiply_vector( mat4x4 &m, vec3d &i) {
        vec3d v;
        v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
		v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
		v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
		v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
        return v;
    }

    mat4x4 matrix_make_identity() {
        mat4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = 1.0f;
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 matrix_make_rotation_x( float32 fAngleRad ) {
        mat4x4 matrix;
        matrix.m[0][0] = 1;
        matrix.m[1][1] = cosf(fAngleRad * 0.5f);
        matrix.m[1][2] = sinf(fAngleRad * 0.5f);
        matrix.m[2][1] = -sinf(fAngleRad * 0.5f);
        matrix.m[2][2] = cosf(fAngleRad * 0.5f);
        matrix.m[3][3] = 1;
        return matrix;
    }

    mat4x4 matrix_make_rotation_y( float32 fAngleRad )
	{
		mat4x4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[0][2] = sinf(fAngleRad);
		matrix.m[2][0] = -sinf(fAngleRad);
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

    mat4x4 matrix_make_rotation_z( float32 fAngleRad ) {
        mat4x4 matrix;
        matrix.m[0][0] = cosf(fAngleRad);
        matrix.m[0][1] = sinf(fAngleRad);
        matrix.m[1][0] = -sinf(fAngleRad);
        matrix.m[1][1] = cosf(fAngleRad);
        matrix.m[2][2] = 1;
        matrix.m[3][3] = 1;
        return matrix;
    }

    mat4x4 matrix_make_translation( float32 x, float32 y, float32 z) {
        mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;
		return matrix;
    }

    mat4x4 matrix_make_projection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
	{
		float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
		mat4x4 matrix;
		matrix.m[0][0] = fAspectRatio * fFovRad;
		matrix.m[1][1] = fFovRad;
		matrix.m[2][2] = fFar / (fFar - fNear);
		matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matrix.m[2][3] = 1.0f;
		matrix.m[3][3] = 0.0f;
		return matrix;
	}

    mat4x4 matrix_multiply_matrix(mat4x4 &m1, mat4x4 &m2)
	{
		mat4x4 matrix;
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
		return matrix;
	}
    
    mat4x4 matrix_point_at(vec3d &pos, vec3d &target, vec3d &up) {
        // Calculate new forward direction
        vec3d newForward = vector_sub(target, pos);
		newForward = vector_normalize(newForward);

		// Calculate new Up direction
		vec3d a = vector_mul(newForward, vector_dot_product(up, newForward));
		vec3d newUp = vector_sub(up, a);
		newUp = vector_normalize(newUp);

		// New Right direction is easy, its just cross product
		vec3d newRight = vector_cross_product(newUp, newForward);

		// Construct Dimensioning and Translation Matrix	
		mat4x4 matrix;
		matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
		return matrix;
    }

    mat4x4 matrix_quick_inverse(mat4x4 &m) // Only for Rotation/Translation Matrices
	{
		mat4x4 matrix;
		matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}


    vec3d vector_add( vec3d &v1, vec3d &v2) {
        return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    }

    vec3d vector_sub( vec3d &v1, vec3d &v2) {
        return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    }

    vec3d vector_mul( vec3d &v, float32 k) {
        return { v.x * k, v.y * k, v.z * k };
    }

    vec3d vector_div( vec3d &v, float32 k) {
        return { v.x / k, v.y / k, v.z / k };
    }

    float32 vector_dot_product( vec3d &v1, vec3d &v2 ) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    float32 vector_length( vec3d &v ) {
        return sqrtf( vector_dot_product( v, v ));
    }

    vec3d vector_normalize( vec3d &v ) {
        float32 l = vector_length( v );
        return { v.x / l, v.y / l, v.z / l };
    }

    vec3d vector_cross_product( vec3d &v1, vec3d &v2 ) {
        vec3d v;
        v.x = v1.y * v2.z - v1.z * v2.y;
		v.y = v1.z * v2.x - v1.x * v2.z;
		v.z = v1.x * v2.y - v1.y * v2.x;
        return v;
    }

    vec3d vector_intersect_plane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd)
	{
		plane_n = vector_normalize(plane_n);
		float plane_d = -vector_dot_product(plane_n, plane_p);
		float ad = vector_dot_product(lineStart, plane_n);
		float bd = vector_dot_product(lineEnd, plane_n);
		float t = (-plane_d - ad) / (bd - ad);
		vec3d lineStartToEnd = vector_sub(lineEnd, lineStart);
		vec3d lineToIntersect = vector_mul(lineStartToEnd, t);
		return vector_add(lineStart, lineToIntersect);
	}

    int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2)
	{
		// Make sure plane normal is indeed normal
		plane_n = vector_normalize(plane_n);

		// Return signed shortest distance from point to plane, plane normal must be normalised
		auto dist = [&](vec3d &p)
		{
			vec3d n = vector_normalize(p);
			return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - vector_dot_product(plane_n, plane_p));
		};

		// Create two temporary storage arrays to classify points either side of plane
		// If distance sign is positive, point lies on "inside" of plane
		vec3d* inside_points[3];  int nInsidePointCount = 0;
		vec3d* outside_points[3]; int nOutsidePointCount = 0;

		// Get signed distance of each point in triangle to plane
		float d0 = dist(in_tri.p[0]);
		float d1 = dist(in_tri.p[1]);
		float d2 = dist(in_tri.p[2]);

		if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; }
		else { outside_points[nOutsidePointCount++] = &in_tri.p[0]; }
		if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[1]; }
		else { outside_points[nOutsidePointCount++] = &in_tri.p[1]; }
		if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[2]; }
		else { outside_points[nOutsidePointCount++] = &in_tri.p[2]; }

		// Now classify triangle points, and break the input triangle into 
		// smaller output triangles if required. There are four possible
		// outcomes...

		if (nInsidePointCount == 0)
		{
			// All points lie on the outside of plane, so clip whole triangle
			// It ceases to exist

			return 0; // No returned triangles are valid
		}

		if (nInsidePointCount == 3)
		{
			// All points lie on the inside of plane, so do nothing
			// and allow the triangle to simply pass through
			out_tri1 = in_tri;

			return 1; // Just the one returned original triangle is valid
		}

		if (nInsidePointCount == 1 && nOutsidePointCount == 2)
		{
			// Triangle should be clipped. As two points lie outside
			// the plane, the triangle simply becomes a smaller triangle

			// Copy appearance info to new triangle
			out_tri1.color =  in_tri.color;

			// The inside point is valid, so keep that...
			out_tri1.p[0] = *inside_points[0];

			// but the two new points are at the locations where the 
			// original sides of the triangle (lines) intersect with the plane
			out_tri1.p[1] = vector_intersect_plane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
			out_tri1.p[2] = vector_intersect_plane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

			return 1; // Return the newly formed single triangle
		}

		if (nInsidePointCount == 2 && nOutsidePointCount == 1)
		{
			// Triangle should be clipped. As two points lie inside the plane,
			// the clipped triangle becomes a "quad". Fortunately, we can
			// represent a quad with two new triangles

			// Copy appearance info to new triangles
			out_tri1.color =  in_tri.color;

			out_tri2.color =  in_tri.color;

			// The first triangle consists of the two inside points and a new
			// point determined by the location where one side of the triangle
			// intersects with the plane
			out_tri1.p[0] = *inside_points[0];
			out_tri1.p[1] = *inside_points[1];
			out_tri1.p[2] = vector_intersect_plane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

			// The second triangle is composed of one of he inside points, a
			// new point determined by the intersection of the other side of the 
			// triangle and the plane, and the newly created point above
			out_tri2.p[0] = *inside_points[1];
			out_tri2.p[1] = out_tri1.p[2];
			out_tri2.p[2] = vector_intersect_plane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

			return 2; // Return two newly formed triangles which form a quad
		}
	}

    void calcMatProj() {
        float32 fNear = 0.1f;
        float32 fFar = 1000.0f;
        float32 fFov = 90.0f;

        float32 fAspectRatio = (float32)framebuffer_height / (float32)framebuffer_width;
        float32 fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f *  3.14159f);

        matProj.m[0][0] = fAspectRatio * fFovRad;
        matProj.m[1][1] = fFovRad;
        matProj.m[2][2] = fFar / (fFar - fNear);
        matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
        matProj.m[2][3] = 1.0;
        matProj.m[3][3] = 0.0;
    }

    void draw_line( float32 x1, float32 y1 , float x2, float32 y2){
    }

    void draw_triangle ( const triangle& tri ) {
        draw_line( tri.p[0].x, tri.p[0].y, tri.p[1].x, tri.p[1].y );
        draw_line( tri.p[1].x, tri.p[1].y, tri.p[2].x, tri.p[2].y );
        draw_line( tri.p[2].x, tri.p[2].y, tri.p[0].x, tri.p[0].y );
    }

    void draw_triangle_filled ( const triangle& tri ) {
    }

    void get_color_by_lum(float32 lum, color3f& out_color ) {
        out_color.r = lum;
        out_color.g = lum;
        out_color.b = lum;
    }


public:

    const float DEG2RAD = 3.14159 / 180;
    float radius = 0.25;

    float32 sc = 1.0;
    GLFWwindow* window;

    unsigned int vertexShader;
    unsigned int fragmentShader_1;
    unsigned int fragmentShader_2;
    unsigned int shaderProgram_1;
    unsigned int shaderProgram_2;

    Shader ourShader;

    unsigned int VBOs[2];
    unsigned int VAOs[2];
    unsigned int EBO;

    GraphicsHandle() {
        
        //meshCube.load_from_object_file("../obj/teapot_minusz.obj");
        
    }

    ~GraphicsHandle() {
        glDeleteVertexArrays(2, VAOs);
        glDeleteBuffers(2, VBOs);
    }

    void init() {

        ourShader = Shader("../shaders/3.3.shader.vs", "../shaders/3.3.shader.fs");

        int  success;
        char infoLog[512];

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float t1_vertices[] = {
            // positions         // colors
            0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
            0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
        };
        
        glGenBuffers( 2, VBOs );
        glGenVertexArrays( 2, VAOs );

        // Setup for this triangle.
        //. Bind to first VAO (position attribute)
        glBindVertexArray( VAOs[0] );
        glBindBuffer( GL_ARRAY_BUFFER, VBOs[0] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( t1_vertices ), t1_vertices, GL_STATIC_DRAW );
        glVertexAttribPointer(0,        // Location of the vertex position vertex attribute
                              3,        // Size of the vertex attribute (3 coordinates);
                              GL_FLOAT, // Type of data
                              GL_FALSE, // If we want the data to be normalized.
                              6 * sizeof( float32 ), // Space between consecutive vertex attribute first positions.
                              (void*) 0 // Where the position data begins in the buffer.
                              );
        glEnableVertexAttribArray( 0 );
        
        //. Bind to second VAO (color attribute)
        glVertexAttribPointer(1,        // attribute location 1
                              3,        // 3 values
                              GL_FLOAT, // value of type float
                              GL_FALSE, // not normalize
                              6 * sizeof( float32 ), // We have to stride 6 floats between beginnings of color values
                              (void*) (3*sizeof(float)) // Color attribute starts after this offset.
                              );
        glEnableVertexAttribArray( 1 );
        // glBindVertexArray( 0 ); // No need to unbind as we bind the next line.
        
        glEnableVertexAttribArray(0); 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }

    

    
    // Updates graphics.
    FRESULT Update( std::vector<Player::PlayerState*>& player_states, bool8 state_got, float32 delta_time ) {


        if ( !window ) {
            return FRESULT(FR_FAILURE);
        }
        else {

            // state-setting function
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            // state-using function
            glClear(GL_COLOR_BUFFER_BIT);


            // Draw first triangle using data from the first VAO
            ourShader.use();
            glBindVertexArray( VAOs[0] );
            glDrawArrays(GL_TRIANGLES, 0, 3);

            if ( history_mode_on ) {
                render_players_with_history( player_states, state_got );
            }
            else {
                render_players( player_states, state_got );
            }
            
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        return FRESULT(FR_OK);
    }
    
    void history_mode_toggle() {
        history_mode_on = !history_mode_on;

        if ( !history_mode_on ) {
            // Luckily std::map destructs any comp
            player_positions.clear();
        }
    }

private:

    struct vector2f {
        float64 x, y;
    };

    struct client_history_pos : vector2f {
        bool8 state_got;
    };

    using history_pos = std::vector<client_history_pos>;

    std::map<uint32, history_pos> player_positions;

    bool8 history_mode_on = true;
    size_t max_history_len = 100;


    
    void render_player(Player::PlayerState* ps, bool8 state_got) {

        Rect_w rect = Rect_w(ps->x, ps->y, player_width, player_height);
        rect.render( state_got );

    }

    void render_players(std::vector<Player::PlayerState*>& player_states, bool8 state_got) {

        for( size_t i = 0; i < player_states.size(); i++) {
            render_player(player_states[i], state_got);
        }

    }

    void render_players_with_history(std::vector<Player::PlayerState*>& player_states, bool8 state_got) {

        for(int i = 0; i < player_states.size(); i++) {

            uint32 player_id = player_states[i]->id;

            // Add player to history map.
            if ( player_positions.count( player_id ) < 1 ) {
                history_pos hpos;
                player_positions.insert( std::pair<uint32, history_pos>(player_id, hpos) );
            }

            if ( player_positions.count( player_id )) {

                // Add new position to history
                player_positions[player_id].push_back( client_history_pos{ { player_states[i]->x, player_states[i]->y }, state_got } );

                // Delete oldest history pos.
                if ( player_positions[player_id].size() > max_history_len ) {
                    player_positions[player_id].erase( player_positions[player_id].begin() );
                }
            }

            // Draw history.
            for( auto const& pos : player_positions[ player_id ]) {
                Rect_w rect = Rect_w(pos.x, pos.y, player_width, player_height);
                rect.render( pos.state_got );
            }
        }
    }

};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

FRESULT create_window(GraphicsHandle& handle) {
    
    handle.window = glfwCreateWindow(window_coord_width, window_coord_width, "Well hello there", NULL, NULL);

    if (!handle.window) {
        glfwTerminate();
        return FRESULT(FR_FAILURE);
    }

    glfwMakeContextCurrent(handle.window);
    
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(handle.window, framebuffer_size_callback); 

    return FRESULT(FR_OK);
}

FRESULT init() {

    if ( !glfwInit() ) {
        return FRESULT(FR_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    return FRESULT(FR_OK);
    
};

FRESULT terminate() {
    glfwTerminate();
    return FRESULT(FR_OK);

};

}