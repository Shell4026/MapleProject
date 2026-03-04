#version 430 core

Shader "Red Shader"
{
	Pass
	{
		LightingPass "Forward"
		Stage Vertex
		{
			void main()
			{
				gl_Position = MATRIX_PROJ * MATRIX_VIEW * MATRIX_MODEL * vec4(VERTEX, 1.0);
			}
		}
		Stage Fragment
		{
			layout(location = 0) out vec4 outColor;

			void main() 
			{
				outColor = vec4(1.00, 0.4, 0.4, 1.0);
			}
		}
	}
}