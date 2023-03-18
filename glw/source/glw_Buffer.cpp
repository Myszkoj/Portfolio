#include "..//include/glw_Buffer.h"


namespace glw
{
	Buffer::Bindings Buffer::bindings =	{ Binding{GL_ARRAY_BUFFER,				GL_NONE}
										, Binding{GL_ATOMIC_COUNTER_BUFFER,		GL_NONE}
										, Binding{GL_COPY_READ_BUFFER,			GL_NONE}
										, Binding{GL_COPY_WRITE_BUFFER,			GL_NONE}
										, Binding{GL_DISPATCH_INDIRECT_BUFFER,	GL_NONE}
										, Binding{GL_DRAW_INDIRECT_BUFFER,		GL_NONE}
										, Binding{GL_ELEMENT_ARRAY_BUFFER,		GL_NONE}
										, Binding{GL_PIXEL_PACK_BUFFER,			GL_NONE}
										, Binding{GL_PIXEL_UNPACK_BUFFER,		GL_NONE}
										, Binding{GL_QUERY_BUFFER,				GL_NONE}
										, Binding{GL_SHADER_STORAGE_BUFFER,		GL_NONE}
										, Binding{GL_TEXTURE_BUFFER,			GL_NONE}
										, Binding{GL_TRANSFORM_FEEDBACK_BUFFER,	GL_NONE}
										, Binding{GL_UNIFORM_BUFFER,			GL_NONE}};
}