//
// Wavefront .obj viewer for WebGL
// Anton Gerdelan 23 Dec 2014
// antongerdelan.net
//
// TODO parse_obj_into_vbos

var canvas;
var obj_fn = "meshes/suzanne.obj"
//var img = "textures/checkerboard.png"
var vs_url = "shaders/basic.vert";
var fs_url = "shaders/basic.frag";
var axis_vs_url = "shaders/axis.vert";
var axis_fs_url = "shaders/axis.frag";
var tex;
var sp;
var M_loc, V_loc, P_loc;
var M;
var deg = 0.0;
var g_axis_vp_vbo, g_axis_vc_vbo;
var g_axis_P_loc, g_axis_V_loc;
var g_axis_dirty;
var q;
var g_axis_v = [0.0, 1.0, 0.0];

var g_time_step_size_s = 1.0 / 50.0; // smallest fixed-step update in seconds

function axis_changed_cb () {
	g_axis_dirty = true;
}

function main () {
	canvas = document.getElementById ("canvas");
	gl = WebGLUtils.setupWebGL (canvas);
	//tex = create_texture_from_file (img);
	parse_obj_into_vbos (obj_fn);
	sp = load_shaders_from_files (vs_url, fs_url);
	gl.bindAttribLocation (sp, 0, "vp");
	gl.bindAttribLocation (sp, 1, "vt");
	gl.bindAttribLocation (sp, 2, "vn");
	gl.linkProgram (sp);
	M_loc = get_uniform_loc (sp, "M");
	P_loc = get_uniform_loc (sp, "P");
	V_loc = get_uniform_loc (sp, "V");

	{
		g_axis_sp = load_shaders_from_files (axis_vs_url, axis_fs_url);
		gl.bindAttribLocation (g_axis_sp, 0, "vp");
		gl.bindAttribLocation (g_axis_sp, 1, "vc");
		gl.linkProgram (g_axis_sp);
		g_axis_P_loc = get_uniform_loc (g_axis_sp, "P");
		g_axis_V_loc = get_uniform_loc (g_axis_sp, "V");
	}
	{
		g_axis_vp_vbo = gl.createBuffer ();
		var vp = [
			0.0, 3.0, 0.0,
			0.0, -3.0, 0.0
		];
		gl.bindBuffer (gl.ARRAY_BUFFER, g_axis_vp_vbo);
		gl.bufferData (gl.ARRAY_BUFFER, new Float32Array (vp), gl.STATIC_DRAW);
		g_axis_vc_vbo = gl.createBuffer ();
		var vc = [
			0.0, 1.0, 0.0,
			0.0, 0.2, 0.0
		];
		gl.bindBuffer (gl.ARRAY_BUFFER, g_axis_vc_vbo);
		gl.bufferData (gl.ARRAY_BUFFER, new Float32Array (vc), gl.STATIC_DRAW);
	}
	
	q = quat_from_axis_deg (0.0, 0.0, 1.0, 0.0);
	M = identity_mat4 ();
	var V = translate_mat4 (identity_mat4 (), [0.0, 0.0, -5.0]);
	var P = perspective (67.0, 1.0, 0.1, 100.0);
	gl.useProgram (sp);
	gl.uniformMatrix4fv (M_loc, gl.FALSE, new Float32Array (M));
	gl.uniformMatrix4fv (V_loc, gl.FALSE, new Float32Array (V));
	gl.uniformMatrix4fv (P_loc, gl.FALSE, new Float32Array (P));
	gl.useProgram (g_axis_sp);
	gl.uniformMatrix4fv (g_axis_V_loc, gl.FALSE, new Float32Array (V));
	gl.uniformMatrix4fv (g_axis_P_loc, gl.FALSE, new Float32Array (P));
	gl.cullFace (gl.BACK);
	gl.frontFace (gl.CCW);
	gl.enable (gl.CULL_FACE);
	gl.enable (gl.DEPTH_TEST);
	gl.disable (gl.BLEND);
	gl.clearColor (0.1, 0.1, 0.3, 1.0);
	//gl.clearColor (1.0, 1.0, 1.0, 1.0);
	gl.viewport (0, 0, canvas.clientWidth, canvas.clientHeight);
	main_loop ();
}

function main_loop () {
	draw ();
	
	// TODO actual timer
	deg += 1.0 / 50.0 * 50.0

	if (deg >= 360.0) {
		deg -= 360.0;
	}
	var quat_angle_el = document.getElementById ("quat_angle");
	quat_angle_el.value = deg.toString ();

	if (g_axis_dirty) {
		var x = parseInt (document.getElementById ("quat_x_axis_input").value);
		var y = parseInt (document.getElementById ("quat_y_axis_input").value);
		var z = parseInt (document.getElementById ("quat_z_axis_input").value);
		g_axis_v = normalise_vec3 ([x, y, z]);
		var vp = [
			g_axis_v[0] * 3.0, g_axis_v[1] * 3.0, g_axis_v[2] * 3.0,
			-g_axis_v[0] * 3.0, -g_axis_v[1] * 3.0, -g_axis_v[2] * 3.0
		];
		gl.bindBuffer (gl.ARRAY_BUFFER, g_axis_vp_vbo);
		gl.bufferData (gl.ARRAY_BUFFER, new Float32Array (vp), gl.STATIC_DRAW);
		var normd_el = document.getElementById ("normd");
		normd_el.innerHTML = "normalise(axis) = [" + g_axis_v[0].toFixed(2) + ", " +
			g_axis_v[1].toFixed(2) + ", " + g_axis_v[2].toFixed(2) + "]<br />";
		g_axis_dirty = false;
	}

	q = quat_from_axis_deg (deg, g_axis_v[0], g_axis_v[1], g_axis_v[2]);

	var versor_el = document.getElementById ("versor");
	versor_el.innerHTML = "versor = [<br />" +
	"&nbsp;&nbsp;cos (angle),<br />" +
	"&nbsp;&nbsp;x * sin (0.5 * angle),<br />" +
	"&nbsp;&nbsp;y * sin (0.5 * angle),<br />" +
	"&nbsp;&nbsp;z * sin (0.5 * angle)<br />" +
	"]<br />" +
	"versor = [" + q[0].toFixed(2) + ", " + q[1].toFixed(2) + ", " +
	q[2].toFixed(2) + ", " + q[3].toFixed(2) + "]";

	M = quat_to_mat4 (q);

	var str = "<br />" + "&nbsp;&nbsp;| " + M[0].toPrecision (2) + " " + M[4].toFixed (2) + " " +
		M[8].toFixed (2) + " " + M[12].toFixed (2) + " |<br />" +
		"&nbsp;&nbsp;| " + M[1].toFixed (2) + " " + M[5].toFixed (2) + " " +
		M[9].toFixed (2) + " " + M[13].toFixed (2) + " |<br />" +
		"= | " + M[2].toFixed (2) + " " + M[6].toFixed (2) + " " +
		M[10].toFixed (2) + " " + M[14].toFixed (2) + " |<br />" +
		"&nbsp;&nbsp;| " + M[3].toFixed (2) + " " + M[7].toFixed (2) + " " +
		M[11].toFixed (2) + " " + M[15].toFixed (2) + " |";

	var qtoM_el = document.getElementById ("qtoM");
	qtoM_el.innerHTML = "mat4 R = quaternion_to_matrix (versor)<br />" + str;

	// this function is from webgl-utils
	window.requestAnimFrame (main_loop, canvas);
}

function draw () {
	gl.clear (gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);


	gl.useProgram (sp);
	gl.uniformMatrix4fv (M_loc, gl.FALSE, new Float32Array (M));
	//gl.activeTexture (gl.TEXTURE0);
	//gl.bindTexture (gl.TEXTURE_2D, tex);
	gl.bindBuffer (gl.ARRAY_BUFFER, vbo_vp);
	gl.vertexAttribPointer (0, 3, gl.FLOAT, false, 0, 0);
	gl.bindBuffer (gl.ARRAY_BUFFER, vbo_vt);
	gl.vertexAttribPointer (1, 2, gl.FLOAT, false, 0, 0);
	gl.bindBuffer (gl.ARRAY_BUFFER, vbo_vn);
	gl.vertexAttribPointer (2, 3, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray (0);
	gl.enableVertexAttribArray (1);
	gl.enableVertexAttribArray (2);
	gl.drawArrays (gl.TRIANGLES, 0, pc);
	gl.disableVertexAttribArray (0);
	gl.disableVertexAttribArray (1);
	gl.disableVertexAttribArray (2);

	

	gl.useProgram (g_axis_sp);
	gl.bindBuffer (gl.ARRAY_BUFFER, g_axis_vp_vbo);
	gl.vertexAttribPointer (0, 3, gl.FLOAT, false, 0, 0);
	gl.bindBuffer (gl.ARRAY_BUFFER, g_axis_vc_vbo);
	gl.vertexAttribPointer (1, 3, gl.FLOAT, false, 0, 0);
	gl.enableVertexAttribArray (0);
	gl.enableVertexAttribArray (1);
	gl.drawArrays (gl.LINES, 0, 2);
	gl.disableVertexAttribArray (0);
	gl.disableVertexAttribArray (1);
}
