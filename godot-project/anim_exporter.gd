class_name AnimExporter
extends AnimationPlayer


func to_c_str(v: Variant) -> String:
	if typeof(v) == TYPE_VECTOR2:
		return "glm::vec2{%ff, %ff}" % [v.x, v.y]
	elif typeof(v) == TYPE_FLOAT:
		return "%ff" % v
	elif typeof(v) == TYPE_COLOR:
		return "glm::vec4{%ff, %ff, %ff, %ff}" % [v.r, v.g, v.b, v.a]
	return var_to_str(v)




var a: Animation;

func print_track_(anim_var_name: String, name : String, property_name : String, c_type_name : String):
	var track_index := a.find_track(name + ":" + property_name, Animation.TYPE_VALUE)
	if track_index == -1:
		return
	
	var c_name := NodePath(name).get_name(NodePath(name).get_name_count() - 1)
	
	var s : String = "";
	
	s += "static %s %s_sprite_tracks_%s_%s_values[] = {" % [c_type_name, anim_var_name, c_name, property_name]
	
	if track_index != -1:
		var key_count := a.track_get_key_count(track_index)
		for i in key_count:
			var value = a.track_get_key_value(track_index, i)
			s += to_c_str(value)
			if i != key_count - 1: s += ", "
	
	s += "};"
	
	
	print(s)
	s = ""
	
	
	
	s += "static float %s_sprite_tracks_%s_%s_times[] = {" % [anim_var_name, c_name, property_name]
	
	if track_index != -1:
		var key_count := a.track_get_key_count(track_index)
		for i in key_count:
			var time := a.track_get_key_time(track_index, i)
			s += to_c_str(time)
			if i != key_count - 1: s += ", "
	
	s += "};"
	
	print(s)
	



func print_sprite_tracks(anim_var_name: String, sprite_tracks: Array[String], sprite_indices : Array[String]):
	for name in sprite_tracks:
		var c_name := NodePath(name).get_name(NodePath(name).get_name_count() - 1)
		
		print_track_(anim_var_name, name, "position", "glm::vec2")
		print_track_(anim_var_name, name, "modulate", "glm::vec4")
		print_track_(anim_var_name, name, "scale",    "glm::vec2")
	
	print("static SpriteTrack %s_sprite_tracks[] = {" % anim_var_name)
	
	var i := 0;
	for name in sprite_tracks:
		var c_name := NodePath(name).get_name(NodePath(name).get_name_count() - 1)
		
		print("	{ // %s" % c_name)
		
		
		print("		/* .sprite_index = */ %s," % [sprite_indices[i]])
		
		
		var values_array_name := "%s_sprite_tracks_%s_%s_values" % [anim_var_name, c_name, "position"]
		var times_array_name  := "%s_sprite_tracks_%s_%s_times"  % [anim_var_name, c_name, "position"]
		
		var track_index := a.find_track(name + ":position", Animation.TYPE_VALUE)
		if track_index == -1:
			print("		/* .position     = */ {},")
		else:
			var discrete : String
			if a.value_track_get_update_mode(track_index) == Animation.UPDATE_CONTINUOUS:
				discrete = "false"
			elif a.value_track_get_update_mode(track_index) == Animation.UPDATE_DISCRETE:
				discrete = "true"
			else:
				assert(false)
			
			print("		/* .position     = */ {%s, ArrayLength(%s), %s, ArrayLength(%s), %s}," % [values_array_name, values_array_name, times_array_name, times_array_name, discrete])
		
		
		
		
		
		
		values_array_name = "%s_sprite_tracks_%s_%s_values" % [anim_var_name, c_name, "modulate"]
		times_array_name  = "%s_sprite_tracks_%s_%s_times"  % [anim_var_name, c_name, "modulate"]
		
		track_index = a.find_track(name + ":modulate", Animation.TYPE_VALUE)
		if track_index == -1:
			print("		/* .modulate     = */ {},")
		else:
			var discrete : String
			if a.value_track_get_update_mode(track_index) == Animation.UPDATE_CONTINUOUS:
				discrete = "false"
			elif a.value_track_get_update_mode(track_index) == Animation.UPDATE_DISCRETE:
				discrete = "true"
			else:
				assert(false)
			
			print("		/* .modulate     = */ {%s, ArrayLength(%s), %s, ArrayLength(%s), %s}," % [values_array_name, values_array_name, times_array_name,times_array_name, discrete])
		
		
		
		
		
		
		values_array_name = "%s_sprite_tracks_%s_%s_values" % [anim_var_name, c_name, "scale"]
		times_array_name  = "%s_sprite_tracks_%s_%s_times"  % [anim_var_name, c_name, "scale"]
		
		track_index = a.find_track(name + ":scale", Animation.TYPE_VALUE)
		if track_index == -1:
			print("		/* .scale        = */ {},")
		else:
			var discrete : String
			if a.value_track_get_update_mode(track_index) == Animation.UPDATE_CONTINUOUS:
				discrete = "false"
			elif a.value_track_get_update_mode(track_index) == Animation.UPDATE_DISCRETE:
				discrete = "true"
			else:
				assert(false)
			
			print("		/* .scale        = */ {%s, ArrayLength(%s), %s, ArrayLength(%s), %s}," % [values_array_name, values_array_name, times_array_name,times_array_name, discrete])
		
		
		
		
		
		
		
		
		
		print("	},")
		i += 1;
	
	
	print("};")

func print_anim(a_: Animation, anim_var_name: String, sprite_tracks: Array[String], sprite_indices : Array[String]):
	
	a = a_;
	
	print("#include \"animation.h\"")
	print("#include \"assets.h\"")
	
	print_sprite_tracks(anim_var_name, sprite_tracks, sprite_indices)
	
	
	
	print("AnimData %s = {" % anim_var_name)
	
	print("	/* .sprite_tracks     = */ %s_sprite_tracks," % anim_var_name)
	print("	/* .num_sprite_tracks = */ ArrayLength(%s_sprite_tracks)," % anim_var_name)
	print("	/* .length            = */ %s," % to_c_str(a.get_length()))
	
	print("};")
