uniform sampler2D background;
uniform sampler2D brush;
uniform float fadeout;

uniform int mode;

void main()
{
	vec2 uv = gl_TexCoord[ 0 ].st;

	vec4 bc = texture2D( background, uv );
	vec4 c = texture2D( brush, uv );

	// premultiply alpha
	c *= c.a;
	bc *= bc.a;

	vec3 blend;
	if ( mode == 0 )
		blend = min( bc.rgb, c.rgb ); // darken
	else
		blend = max( bc.rgb, c.aaa ); // erase

	// fade out
	vec3 outc = mix( vec3( 1, 1, 1 ), blend, fadeout );
	gl_FragColor = vec4( outc, 1. );
}

