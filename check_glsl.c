#include <GL/glx.h>
#include <GL/glxext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int vAttributes[] = {
	None
};

static int cAttributes[] = {
	GLX_CONTEXT_MAJOR_VERSION_ARB , 4 ,
	GLX_CONTEXT_MINOR_VERSION_ARB , 1 ,
	GLX_CONTEXT_PROFILE_MASK_ARB ,
		GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB ,
	0
};

static int bAttributes[] = {
	GLX_PBUFFER_WIDTH , 32 ,
	GLX_PBUFFER_HEIGHT , 32 ,
	None
};


#define gglp(t,n) \
	(t)glXGetProcAddress( "gl" n )


static int errmsg( char const* msg )
{
	fprintf( stderr , "%s\n" , msg );
	return 0;
}


static Display * xDisplay;
static GLXContext glxContext;
static GLXPbuffer pBuffer;
static PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
static PFNGLDELETEPROGRAMPROC glDeleteProgram;


static void debugCallback( GLenum source , GLenum type , GLuint id ,
		GLenum severity , GLsizei length , GLchar const* message ,
		void const* userParam )
{
	fprintf( stderr , "%x %x %x %x %s\n" , source , type , id , severity ,
			message );
}


static int initContext( )
{
	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribs;
	GLXFBConfig * fbc;
	int fbcount;

	xDisplay = XOpenDisplay( 0 );
	if ( !xDisplay ) {
		return errmsg( "Could not open display" );
	}

	fbc = glXChooseFBConfig( xDisplay , DefaultScreen( xDisplay ) ,
			vAttributes , &fbcount );
	if ( !fbc ) {
		return errmsg( "No framebuffer config" );
	}

	glXCreateContextAttribs = gglp( PFNGLXCREATECONTEXTATTRIBSARBPROC ,
		"XCreateContextAttribsARB" );

	glxContext = glXCreateContextAttribs( xDisplay , fbc[ 0 ] , NULL ,
			GL_TRUE , cAttributes );
	if ( !glxContext ) {
		return errmsg( "Could not create GL3.0+ context" );
	}

	pBuffer = glXCreatePbuffer( xDisplay , fbc[ 0 ] , bAttributes );
	if ( !pBuffer ) {
		return errmsg( "Could not create PBuffer" );
	}

	XFree( fbc );
	XSync( xDisplay , False );
	if ( ! glXMakeContextCurrent( xDisplay , pBuffer , pBuffer ,
				glxContext ) ) {
		return errmsg( "Could not activate context" );
	}

	glCreateShaderProgramv = gglp( PFNGLCREATESHADERPROGRAMVPROC ,
			"CreateShaderProgramv" );
	glGetProgramInfoLog = gglp( PFNGLGETPROGRAMINFOLOGPROC ,
			"GetProgramInfoLog" );
	glDeleteProgram = gglp( PFNGLDELETEPROGRAMPROC , "DeleteProgram" );

	return 1;
}


static void killContext( )
{
	glXDestroyPbuffer( xDisplay , pBuffer );
	glXMakeCurrent( xDisplay , 0 , 0 );
	glXDestroyContext( xDisplay , glxContext );
	XCloseDisplay( xDisplay );
}


static GLenum getShaderType( char const* argument )
{
	if ( !strcmp( argument , "v" ) )
		return GL_VERTEX_SHADER;
	if ( !strcmp( argument , "f" ) )
		return GL_FRAGMENT_SHADER;
	if ( !strcmp( argument , "g" ) )
		return GL_GEOMETRY_SHADER;
	if ( !strcmp( argument , "tc" ) )
		return GL_TESS_CONTROL_SHADER;
	if ( !strcmp( argument , "te" ) )
		return GL_TESS_EVALUATION_SHADER;
	if ( !strcmp( argument , "c" ) )
		return GL_COMPUTE_SHADER;
	fprintf( stderr , "Invalid shader type '%s'\n" , argument );
	return 0;
}

static char * loadFile( char const* name )
{
	FILE * f = fopen( name , "r" );
	if ( !f ) {
		fprintf( stderr , "%s: file not found\n" , name );
		return NULL;
	}

	int sz;
	fseek( f , 0 , SEEK_END );
	sz = ftell( f );
	if ( sz <= 0 ) {
		fprintf( stderr , "%s: fseek failed / empty file\n" , name );
		return NULL;
	}
	fseek( f , 0 , SEEK_SET );

	char * buffer = malloc( sz + 1 );
	if ( !buffer ) {
		fprintf( stderr , "%s: malloc() failed (%d bytes)\n" , name ,
				sz );
		return NULL;
	}
	if ( !fread( buffer , 1 , sz , f ) ) {
		fprintf( stderr , "%s: could not read file contents\n" , name );
		return NULL;
	}
	buffer[ sz ] = 0;
	fclose(f);

	return buffer;
}


void loadShader( GLenum type , char const* file )
{
	GLuint program;
	char * source = loadFile( file );
	if ( source == NULL ) {
		return;
	}

	program = glCreateShaderProgramv( type , 1 , (GLchar const**) &source );
	free( source );

	char infoLog[ 128 * 1024 ];
	int sz , i , j;
	glGetProgramInfoLog( program , sizeof( infoLog ) , &sz , infoLog );

	i = 0;
	while ( i < sz ) {
		j = i;
		while ( j < sz && infoLog[ j ] != '\n' ) {
			j ++;
		}
		if ( j < sz ) {
			infoLog[ j ] = 0;
		}
		fprintf( stderr , "%s: %s\n" , file , &infoLog[ i ] );
		i = j + 1;
	}

	glDeleteProgram( program );
}


int main( int argc , char ** argv )
{
	if ( !initContext( ) ) {
		return 1;
	}

	if ( argc > 1 ) {
		if ( argc < 3 ) {
			fprintf( stderr ,
					"Syntax: %s [v|f|g|tc|te|c] file...\n" ,
					argv[ 0 ] );
			return 1;
		}

		GLenum type = getShaderType( argv[ 1 ] );
		if ( type == 0 ) {
			return 1;
		}

		int i;
		for ( i = 2 ; i < argc ; i ++ ) {
			loadShader( type , argv[ i ] );
		}
	}

	killContext( );
	return 0;
}
