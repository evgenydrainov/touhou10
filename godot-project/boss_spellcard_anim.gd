extends AnimExporter

func _ready() -> void:
	var a := get_animation("new_animation")
	
	var sprite_tracks : Array[String] = [
		"AnimationPlayer/BossCirnoPortrait",
		"AnimationPlayer/White",
		"AnimationPlayer/SpellCardAttackAnimLabel",
		"AnimationPlayer/SpellCardAttackAnimLabel2",
		"AnimationPlayer/SpellCardAttackAnimLabel3",
		"AnimationPlayer/SpellCardAttackAnimLabel4",
	];
	
	var sprite_indices : Array[String] = [
		"tex_boss_cirno_portrait",
		"tex_white",
		"tex_spellcard_attack_anim_label",
		"tex_spellcard_attack_anim_label",
		"tex_spellcard_attack_anim_label",
		"tex_spellcard_attack_anim_label",
	];
	
	print_anim(a, "anim_boss_spellcard", sprite_tracks, sprite_indices)
