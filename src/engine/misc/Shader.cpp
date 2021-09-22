#include "./Shader.h"
#include "./gl_helper.h"
#include "../Macros.h"

namespace RG3GE::Core {
//=============================================================================
// OpenGL Helpers
//-----------------------------------------------------------------------------
//=============================================================================
#ifdef DEBUG_BUILD
    void CheckOGLErr(const char* stmt, const char* fname, int line) {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cout << "OpenGL error " << err << " at " << fname << ":" << line << " => " << stmt << std::endl;
        }
    }
#endif

//=============================================================================
// Shader
//-----------------------------------------------------------------------------
//=============================================================================
	static unsigned int CompileShader(unsigned int type, const std::string& src) {
		unsigned int shader = glCreateShader(type);
		const char* sr = src.c_str();
		int l = (int)src.length();
		glShaderSource(shader, 1, &sr, &l);
		glCompileShader(shader);

		int state;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &state);

		if (state != 1) {
			std::cout << "Could not comile shader type " << src << std::endl;

			int log_size = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
			char* buffer = (char*)alloca(log_size);
			glGetShaderInfoLog(shader, log_size, &log_size, buffer);
                std::cout << buffer << std::endl;
            throw buffer;
		}

		return shader;

	}

    uint32_t CreateShader(const std::string& vs, const std::string& fs) {

        try { 
            uint32_t iVS = CompileShader(GL_VERTEX_SHADER, vs); 
            uint32_t iFS = CompileShader(GL_FRAGMENT_SHADER, fs); 

            uint32_t program = glCreateProgram();

            glAttachShader(program, iVS);
            glAttachShader(program, iFS);
            glLinkProgram(program);

            int link_state;
            glGetProgramiv(program, GL_LINK_STATUS, &link_state);

            glDeleteShader(iVS);
            glDeleteShader(iFS);

            if (link_state != 1) {
                std::cout << "Could not link program type " << std::endl;

                int log_size = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
                char* buffer = (char*)alloca(log_size);
                glGetProgramInfoLog(program, log_size, &log_size, buffer);
                std::cout << buffer << std::endl;

                throw buffer;	
            }

            return program;
        }
        catch(const char* s) { std::cout << "failed to compile shader: " << s << std::endl; }

        return 0;

    }
}
