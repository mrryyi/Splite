#pragma once
#include "pre.h"
#include "game.h"
// Our fancy schmancy graphics handler
#include <GLFW/glfw3.h>
#include <cmath>
#include <fstream>
#include <strstream>
#include <algorithm>

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
        glBegin(GL_POLYGON);

        if ( state_got ) {
            glColor3f(1.0, 0.0, 0.0);
        }
        else {
            glColor3f(1.0, 1.0, 1.0);
        }

        glVertex3f(x_start, y_start, 0.0);
        glVertex3f(x_end, y_start, 0.0);
        glVertex3f(x_end, y_end, 0.0);
        glVertex3f(x_start, y_end, 0.0);
        glEnd();
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
        glBegin(GL_LINES);
        glColor3f(0.0, 0.0, 0.0);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }

    void draw_triangle ( const triangle& tri ) {
        draw_line( tri.p[0].x, tri.p[0].y, tri.p[1].x, tri.p[1].y );
        draw_line( tri.p[1].x, tri.p[1].y, tri.p[2].x, tri.p[2].y );
        draw_line( tri.p[2].x, tri.p[2].y, tri.p[0].x, tri.p[0].y );
    }

    void draw_triangle_filled ( const triangle& tri ) {
        glBegin(GL_POLYGON);
        glColor3f(tri.color.r, tri.color.g, tri.color.b);
        glVertex3f(tri.p[0].x, tri.p[0].y, 0.0);
        glVertex3f(tri.p[1].x, tri.p[1].y, 0.0);
        glVertex3f(tri.p[2].x, tri.p[2].y, 0.0);
        glEnd();
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

    GraphicsHandle() {
        
        meshCube.load_from_object_file("../obj/teapot.obj");

    }



    
    // Updates graphics.
    FRESULT Update( std::vector<Player::PlayerState*>& player_states, bool8 state_got, uint64 delta_time ) {
        
        int8 x_cam_dir = glfwGetKey( window, GLFW_KEY_RIGHT );
        x_cam_dir -= glfwGetKey( window, GLFW_KEY_LEFT );
        int8 y_cam_dir = glfwGetKey( window, GLFW_KEY_UP );
        y_cam_dir -= glfwGetKey( window, GLFW_KEY_DOWN );
        vCamera.x += 0.1f * x_cam_dir;
        vCamera.y += 0.1f * y_cam_dir;

        if ( !window ) {
            return FRESULT(FR_FAILURE);
        }
        else {
            //Setup View
            float ratio;
            int32 width, height;
            int viewport_x = 0;
            int viewport_y = 0;

            glfwGetFramebufferSize(window, &width, &height);

            if (height != framebuffer_height || width != framebuffer_width) {
                printf("Size changed.\n");
                framebuffer_height = height;
                framebuffer_width = width;
                matProj = matrix_make_projection(90.0, (float32) framebuffer_height / (float32) framebuffer_width, 0.1f, 1000.0f);
            }

            // Viewport is, basically, as if we're "moving" where the result
            // of our next thing is. Imagine mapping the result to wherever we're
            // "looking".
            glViewport(viewport_x, viewport_y, width, height);
            glClear(GL_COLOR_BUFFER_BIT);
            
            glColor3f(1.0, 1.0, 1.0);

            // Set up rotation matrices
            mat4x4 matRotZ, matRotX;
            //fTheta += 0.001f * delta_time;

            matRotX = matrix_make_rotation_x( fTheta );
            matRotZ = matrix_make_rotation_z( fTheta );

            mat4x4 matTrans;
            matTrans = matrix_make_translation(0.0f, 0.0f, 5.0f);

            mat4x4 matWorld;
            matWorld = matrix_make_identity();
            matWorld = matrix_multiply_matrix( matRotZ, matRotX );
            matWorld = matrix_multiply_matrix( matWorld, matTrans );

            vLookDir = {0, 0, 1};
            vec3d vUp = {0, 1, 0};
            vec3d vTarget = vector_add( vCamera, vLookDir );

            mat4x4 matCamera = matrix_point_at( vCamera, vTarget, vUp );
            mat4x4 matView = matrix_quick_inverse( matCamera );

            std::vector<triangle> vecTrianglesToRaster;
            
            for( auto tri : meshCube.tris ) {

                triangle triProjected, triTransformed, triViewed;

                triTransformed.p[0] = matrix_multiply_vector( matWorld, tri.p[0] );
                triTransformed.p[1] = matrix_multiply_vector( matWorld, tri.p[1] );
                triTransformed.p[2] = matrix_multiply_vector( matWorld, tri.p[2] );

                // After translation into world, but after projection onto screen, we
                // want to fuck off the triangles that should not be seen.
                vec3d normal, line1, line2;

                // Get lines either side of the triangle
                line1 = vector_sub( triTransformed.p[1], triTransformed.p[0] );
                line2 = vector_sub( triTransformed.p[2], triTransformed.p[0] );

                // Take cross product of lines to get normal to triangle surface
                normal = vector_cross_product( line1, line2 );

                normal = vector_normalize( normal );

                vec3d vCameraRay = vector_sub(triTransformed.p[0], vCamera);

                // If ray is aligned with normal, then triangle is visible.
                if ( vector_dot_product(normal, vCameraRay) < 0.0f )
                {
                    
                    // Illumination
                    vec3d light_direction =  {0.0f, 1.0f, -1.0f};
                    light_direction = vector_normalize( light_direction );
                    float32 dp = vector_dot_product( light_direction, normal );
                    if (dp < 0.1f) dp = 0.1f;

                    get_color_by_lum(dp, triTransformed.color);

                    // Convert world space -> view space
                    triViewed.p[0] = matrix_multiply_vector(matView, triTransformed.p[0]);
                    triViewed.p[1] = matrix_multiply_vector(matView, triTransformed.p[1]);
                    triViewed.p[2] = matrix_multiply_vector(matView, triTransformed.p[2]);

                    // Project triangles from 3D --> 2D
                    triProjected.p[0] = matrix_multiply_vector(matProj, triViewed.p[0]);
                    triProjected.p[1] = matrix_multiply_vector(matProj, triViewed.p[1]);
                    triProjected.p[2] = matrix_multiply_vector(matProj, triViewed.p[2]);

                    triProjected.color = triTransformed.color;

                    // Normalize projected vector.
                    triProjected.p[0] = vector_div(triProjected.p[0], triProjected.p[0].w);
                    triProjected.p[1] = vector_div(triProjected.p[1], triProjected.p[1].w);
                    triProjected.p[2] = vector_div(triProjected.p[2], triProjected.p[2].w);

                    // Offset vertices into visible normalized space.
                    vec3d vOffsetView = { 1.0, 1.0, 0 };
                    triProjected.p[0] = vector_add( triProjected.p[0], vOffsetView );
                    triProjected.p[1] = vector_add( triProjected.p[1], vOffsetView );
                    triProjected.p[2] = vector_add( triProjected.p[2], vOffsetView );

                    triProjected.p[0].x *= 0.5f * (float32) width;
                    triProjected.p[0].y *= 0.5f * (float32) height;
                    triProjected.p[1].x *= 0.5f * (float32) width;
                    triProjected.p[1].y *= 0.5f * (float32) height;
                    triProjected.p[2].x *= 0.5f * (float32) width;
                    triProjected.p[2].y *= 0.5f * (float32) height;

                    vecTrianglesToRaster.push_back(triProjected);

                }
            }

            std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2)
            {
                float32 z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
                float32 z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
                return z1 > z2;
            });

            for ( auto &triProjected : vecTrianglesToRaster) {
                draw_triangle_filled(triProjected);
            }
            
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

FRESULT create_window(GraphicsHandle& handle) {
    
    handle.window = glfwCreateWindow(1000, 1000, "Well hello there", NULL, NULL);

    if (!handle.window) {
        glfwTerminate();
        return FRESULT(FR_FAILURE);
    }

    glfwMakeContextCurrent(handle.window);
    glClearColor(0.0, 0.0, 0.0, 0.0);           // black background
    glMatrixMode(GL_PROJECTION);                // setup viewing projection
    glLoadIdentity();                           // start with identity matrix

    // Oh my god. This is amazing. We DON'T HAVE TO SCALE STUFF MANUALLY!!!!!
    glOrtho(0.0, window_coord_width, 0.0, window_coord_height, -1.0, 10000.0);
    
    glfwSwapInterval(1);

    return FRESULT(FR_OK);
}

FRESULT init() {

    if ( !glfwInit() ) {
        return FRESULT(FR_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    return FRESULT(FR_OK);
    
};

FRESULT terminate() {

    glfwTerminate();
    return FRESULT(FR_OK);

};

}