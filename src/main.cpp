// ReSharper disable CppHidingFunction
#include <Geode/Geode.hpp>

using namespace geode::prelude;

gd::string patchSaveString(gd::string save, auto patcher) {
	size_t pos = 0;
	std::stringstream out;
	bool first = true;
	while (pos < save.size()) {
		if (first) {
			first = false;
		} else {
			out << ',';
		}
		size_t count = 0;
		const int key = std::stoi(gd::string(save.data() + pos, save.size() - pos), &count);
		pos += count + 1;
		out << key << ',';

		size_t val_start = pos;
		char c;
		do {
			c = save[pos++];
		} while (c != ',');

		out << patcher(key, gd::string(save.data() + val_start, std::min(pos - val_start - 1, save.size() - val_start)));
	}

	return out.str();
}

bool precisionPosition = true;
bool precisionRotation = true;
bool precisionScale = true;
bool precisionParams = true;
bool decimalMoveParams = true;
bool sliderInputs = true;

$execute {
	precisionPosition = Mod::get()->getSettingValue<bool>("full-precision-object-position");
	precisionRotation = Mod::get()->getSettingValue<bool>("full-precision-object-rotation");
	precisionScale = Mod::get()->getSettingValue<bool>("full-precision-object-scale");
	precisionParams = Mod::get()->getSettingValue<bool>("full-precision-trigger-parameters");
	decimalMoveParams = Mod::get()->getSettingValue<bool>("allow-decimal-move-parameters");
	sliderInputs = Mod::get()->getSettingValue<bool>("enable-slider-inputs");
	listenForSettingChanges("full-precision-object-position", [](bool value) {
		precisionPosition = value;
	});
	listenForSettingChanges("full-precision-object-rotation", [](bool value) {
		precisionRotation = value;
	});
	listenForSettingChanges("full-precision-object-scale", [](bool value) {
		precisionScale = value;
	});
	listenForSettingChanges("full-precision-trigger-parameters", [](bool value) {
		precisionParams = value;
	});
	listenForSettingChanges("allow-decimal-move-parameters", [](bool value) {
		decimalMoveParams = value;
	});
	listenForSettingChanges("enable-slider-inputs", [](bool value) {
		sliderInputs = value;
	});
}

#include <Geode/modify/GameObject.hpp>
class $modify(PrecisionGameObject, GameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = GameObject::getSaveString(layer);
		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 2:
					if (!precisionPosition) return orig;
					return gd::string(gd::string(fmt::format("{}", m_positionX)));
				case 3:
					if (!precisionPosition) return orig;
					return gd::string(gd::string(fmt::format("{}", m_positionY - 90)));
				case 32:
					if (!precisionScale) return orig;
					return gd::string(gd::string(fmt::format("{}", std::max(m_scaleX, m_scaleY))));
				case 6:
				case 131:
					if (!precisionRotation) return orig;
					return gd::string(gd::string(fmt::format("{}", m_fRotationX)));
				case 132:
					if (!precisionRotation) return orig;
					return gd::string(gd::string(fmt::format("{}", m_fRotationY)));
				case 128:
					if (!precisionScale) return orig;
					return gd::string(gd::string(fmt::format("{}", m_scaleX)));
				case 129:
					if (!precisionScale) return orig;
					return gd::string(gd::string(fmt::format("{}", m_scaleY)));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/EffectGameObject.hpp>
class $modify(PrecisionEffectObject, EffectGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = EffectGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
					//trigger common
				case 10: //also used as random trigger chance
					return gd::string(fmt::format("{}", m_duration));
				case 85:
					return gd::string(fmt::format("{}", m_easingRate));

					//move trigger
				case 28: //also used by camera offset and guide triggers
					return gd::string(fmt::format("{}", m_moveOffset.x));
				case 29: //also used by camera offset and guide triggers
					return gd::string(fmt::format("{}", m_moveOffset.y));
				case 143:
					return gd::string(fmt::format("{}", m_moveModX));
				case 144:
					return gd::string(fmt::format("{}", m_moveModY));

					//rotate trigger
				case 68: //also used by camera rotate trigger
					return gd::string(fmt::format("{}", m_rotationDegrees));
				case 402:
					return gd::string(fmt::format("{}", m_rotationOffset));

					//pulse trigger
				case 45:
					return gd::string(fmt::format("{}", m_fadeInDuration));
				case 46:
					return gd::string(fmt::format("{}", m_holdDuration));
				case 47:
					return gd::string(fmt::format("{}", m_fadeOutDuration));

					//alpha trigger
				case 35:
					return gd::string(fmt::format("{}", m_opacity));

					//shake trigger
				case 75:
					return gd::string(fmt::format("{}", m_shakeStrength));
				case 84:
					return gd::string(fmt::format("{}", m_shakeInterval));

					//follow trigger
				case 72:
					return gd::string(fmt::format("{}", m_followXMod));
				case 73:
					return gd::string(fmt::format("{}", m_followYMod));

					//follow player y trigger
				case 90:
					return gd::string(fmt::format("{}", m_followYSpeed));
				case 91:
					return gd::string(fmt::format("{}", m_followYDelay));
				case 105:
					return gd::string(fmt::format("{}", m_followYMaxSpeed));

					//camera zoom trigger
				case 371: //also used by camera guide
					return gd::string(fmt::format("{}", m_zoomValue));

					//camera mode trigger
				case 114:
					return gd::string(fmt::format("{}", m_cameraPaddingValue));

					//timewarp trigger
				case 120:
					return gd::string(fmt::format("{}", m_timeWarpTimeMod));

					//gravity trigger
				case 148:
					return gd::string(fmt::format("{}", m_gravityValue));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/TransformTriggerGameObject.hpp>
class $modify(PrecisionTransformTrigger, TransformTriggerGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = TransformTriggerGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
					//scale trigger
				case 150:
					return gd::string(fmt::format("{}", m_objectScaleX));
				case 151:
					return gd::string(fmt::format("{}", m_objectScaleY));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/KeyframeAnimTriggerObject.hpp>
class $modify(PrecisionKeyframeAnimTrigger, KeyframeAnimTriggerObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = KeyframeAnimTriggerObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 520:
					return gd::string(fmt::format("{}", m_timeMod));
				case 521:
					return gd::string(fmt::format("{}", m_positionXMod));
				case 545:
					return gd::string(fmt::format("{}", m_positionYMod));
				case 522:
					return gd::string(fmt::format("{}", m_rotationMod));
				case 523:
					return gd::string(fmt::format("{}", m_scaleXMod));
				case 546:
					return gd::string(fmt::format("{}", m_scaleYMod));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/KeyframeGameObject.hpp>
class $modify(PrecisionKeyframeGameObject, KeyframeGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = KeyframeGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 557:
					return gd::string(fmt::format("{}", m_spawnDelay));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/GradientTriggerObject.hpp>
class $modify(PrecisionGradientTrigger, GradientTriggerObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = GradientTriggerObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 456:
					return gd::string(fmt::format("{}", m_previewOpacity));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/CameraTriggerGameObject.hpp>
class $modify(PrecisionCameraTrigger, CameraTriggerGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = CameraTriggerGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 213:
					return gd::string(fmt::format("{}", m_followEasing));
				case 454:
					return gd::string(fmt::format("{}", m_velocityModifier));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/ItemTriggerGameObject.hpp>
class $modify(PrecisionItemTrigger, ItemTriggerGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = ItemTriggerGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 479:
					return gd::string(fmt::format("{}", m_mod1));
				case 483:
					return gd::string(fmt::format("{}", m_mod2));
				case 484:
					return gd::string(fmt::format("{}", m_tolerance));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/SFXTriggerGameObject.hpp>
class $modify(PrecisionSFXTrigger, SFXTriggerGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = SFXTriggerGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 406: //also used by song trigger
					return gd::string(fmt::format("{}", m_volume));
				case 421:
					return gd::string(fmt::format("{}", m_volumeNear));
				case 422:
					return gd::string(fmt::format("{}", m_volumeMedium));
				case 423:
					return gd::string(fmt::format("{}", m_volumeFar));
				case 434:
					return gd::string(fmt::format("{}", m_minInterval));
				case 490:
					return gd::string(fmt::format("{}", m_soundDuration));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/TimerTriggerGameObject.hpp>
class $modify(PrecisionTimerTrigger, TimerTriggerGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = TimerTriggerGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 467:
					return gd::string(fmt::format("{}", m_startTime));
				case 473: //also used by time event trigger
					return gd::string(fmt::format("{}", m_targetTime));
				case 470:
					return gd::string(fmt::format("{}", m_timeMod));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/SpawnTriggerGameObject.hpp>
class $modify(PrecisionSpawnTrigger, SpawnTriggerGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = SpawnTriggerGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 63:
					return gd::string(fmt::format("{}", m_spawnDelay));
				case 556:
					return gd::string(fmt::format("{}", m_delayRange));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/SequenceTriggerGameObject.hpp>
class $modify(PrecisionSequenceTrigger, SequenceTriggerGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = SequenceTriggerGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 437:
					return gd::string(fmt::format("{}", m_minInt));
				case 438:
					return gd::string(fmt::format("{}", m_reset));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/SpawnParticleGameObject.hpp>
class $modify(PrecisionSpawnParticle, SpawnParticleGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = SpawnParticleGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 554:
					return gd::string(fmt::format("{}", m_scale));
				case 555:
					return gd::string(fmt::format("{}", m_scaleVariance));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/RotateGameplayGameObject.hpp>
class $modify(PrecisionRotateGameplay, RotateGameplayGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = RotateGameplayGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 582:
					return gd::string(fmt::format("{}", m_velocityModX));
				case 583:
					return gd::string(fmt::format("{}", m_velocityModY));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/GameOptionsTrigger.hpp>
class $modify(PrecisionGameOptions, GameOptionsTrigger) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = GameOptionsTrigger::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 574:
					return gd::string(fmt::format("{}", m_respawnTime));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/TeleportPortalObject.hpp>
class $modify(PrecisionTeleportPortal, TeleportPortalObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = TeleportPortalObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 348:
					return gd::string(fmt::format("{}", m_redirectForceMin));
				case 349:
					return gd::string(fmt::format("{}", m_redirectForceMax));
				case 350:
					return gd::string(fmt::format("{}", m_redirectForceMod));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/ShaderGameObject.hpp>
class $modify(PrecisionShaderGameObject, ShaderGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = ShaderGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 175:
					return gd::string(fmt::format("{}", m_speed));
				case 176:
					return gd::string(fmt::format("{}", m_strength));
				case 179:
					return gd::string(fmt::format("{}", m_waveWidth));
				case 180:
					return gd::string(fmt::format("{}", m_targetX));
				case 189:
					return gd::string(fmt::format("{}", m_targetY));
				case 181:
					return gd::string(fmt::format("{}", m_fadeIn));
				case 182:
					return gd::string(fmt::format("{}", m_fadeOut));
				case 177:
					return gd::string(fmt::format("{}", m_timeOff));
				case 512:
					return gd::string(fmt::format("{}", m_maxSize));
				case 290:
					return gd::string(fmt::format("{}", m_screenOffsetX));
				case 291:
					return gd::string(fmt::format("{}", m_screenOffsetY));
				case 183:
					return gd::string(fmt::format("{}", m_inner));
				case 191:
					return gd::string(fmt::format("{}", m_outer));

				default:
					return orig;
			}
		});
	}
};
#include <Geode/modify/ForceBlockGameObject.hpp>
class $modify(PrecisionForceBlock, ForceBlockGameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = ForceBlockGameObject::getSaveString(layer);
		if (!precisionParams) return save;

		return patchSaveString(save, [this](const int key, gd::string orig) {
			switch (key) {
				case 149:
					return gd::string(fmt::format("{}", m_force));
				case 526:
					return gd::string(fmt::format("{}", m_minForce));
				case 527:
					return gd::string(fmt::format("{}", m_maxForce));

				default:
					return orig;
			}
		});
	}
};

#include <Geode/modify/SetupTriggerPopup.hpp>
class $modify(PrecisionTriggerPopup, SetupTriggerPopup) {
	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("SetupTriggerPopup::triggerSliderChanged", Priority::Early)) {
			log::error("failed to set hook priority for SetupTriggerPopup::triggerSliderChanged");
		}
	}

	void updateInputNode(int property, float value) override {
		SetupTriggerPopup::updateInputNode(property, value);
		if (!precisionParams) return;

		auto inputNode = (CCTextInputNode*) m_inputNodes->objectForKey(property);
		if (inputNode == nullptr || inputNode->m_textField == nullptr) {
			//log::info("skipped input node due to null text field");
			return;
		}
		const gd::string newStr = gd::string(fmt::format("{}", value));
		const gd::string oldStr = inputNode->getString();
		//log::info("update input node {} -> {}", oldStr, newStr);
		if (std::stof(oldStr) != value || newStr.size() < oldStr.size()) {
			inputNode->setString(newStr);
		}
	}

	CCArray* createValueControlAdvanced(int property,
										gd::string label,
										cocos2d::CCPoint position,
										float scale,
										bool noSlider,
										InputValueType valueType,
										int length,
										bool arrows,
										float sliderMin,
										float sliderMax,
										int page,
										int group,
										GJInputStyle inputStyle,
										int decimalPlaces,
										bool allowDisable) {
		if (!decimalMoveParams) return SetupTriggerPopup::createValueControlAdvanced(property,
																				   label,
																				   position,
																				   scale,
																				   noSlider,
																				   valueType,
																				   length,
																				   arrows,
																				   sliderMin,
																				   sliderMax,
																				   page,
																				   group,
																				   inputStyle,
																				   decimalPlaces,
																				   allowDisable);

		switch (property) {
			case 28:
			case 29:
				//log::info("overrode {} to float input", label);
				valueType = InputValueType::Float;
				decimalPlaces = -1;
				break;
			default: break;
		}
		return SetupTriggerPopup::createValueControlAdvanced(property,
															 label,
															 position,
															 scale,
															 noSlider,
															 valueType,
															 length,
															 arrows,
															 sliderMin,
															 sliderMax,
															 page,
															 group,
															 inputStyle,
															 decimalPlaces,
															 allowDisable);
	}
	float getTruncatedValue(float value, int decimalPlaces) {
		if (!precisionParams) return SetupTriggerPopup::getTruncatedValue(value, std::abs(decimalPlaces));

		if (decimalPlaces != 0) {
			//log::info("overrode truncation of {} from {} places", value, decimalPlaces);
			return value;
		}
		return SetupTriggerPopup::getTruncatedValue(value, std::abs(decimalPlaces));
	}

	void triggerSliderChanged(CCObject* param) {
		if (!precisionParams) return SetupTriggerPopup::triggerSliderChanged(param);

		bool oldDisableTextDelegate = m_disableTextDelegate;
		m_disableTextDelegate = true;

		int property = param->getTag();
		float value = ((SliderThumb*) param)->getValue();
		//log::info("triggerSliderChanged: {}", value);

		value = triggerValueFromSliderValue(property, value);
		auto inputNode = (CCTextInputNode*) m_inputNodes->objectForKey(property);
		if (inputNode != nullptr && inputNode->m_textField != nullptr) {
			int places = inputNode->m_decimalPlaces;
			if (places < 0) places = -places - 1;
			//log::info("truncating to {} places", places);

			if (places < 1) {
				value = (float) (int) value;
			} else {
				float scale = std::powf(10.0, (float) places);
				value = std::roundf(value * scale) / scale;
			}
		}
		valueChanged(property, value);
		updateInputNode(property, value);
		m_disableTextDelegate = oldDisableTextDelegate;
	}
};
void updateTriggers(SetupTriggerPopup* popup, auto updater) {
	if (popup->m_gameObject == nullptr) {
		unsigned int count = popup->m_gameObjects->count();
		for (unsigned int i = 0; i < count; i++) {
			auto object = (EffectGameObject*) popup->m_gameObjects->objectAtIndex(i);
			updater(object);
		}
	} else
		updater(popup->m_gameObject);
}
//why did RobTop inexplicably decide to hardcode the UI logic for these triggers specifically
//instead of using the perfectly good existing facilities that already do the same thing???
#include <Geode/modify/SetupCameraOffsetTrigger.hpp>
class $modify(PrecisionSetupCameraOffset, SetupCameraOffsetTrigger) {
	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("SetupCameraOffsetTrigger::textChanged", Priority::Early)) {
			log::error("failed to set hook priority for SetupCameraOffsetTrigger::textChanged");
		}
	}
	bool init(CameraTriggerGameObject* p0, CCArray* p1) {
		if (!SetupCameraOffsetTrigger::init(p0, p1)) return false;
		if (!precisionParams) return true;

		auto object = m_gameObject;
		if (object == nullptr) {
			if (m_gameObjects == nullptr || m_gameObjects->count() == 0) return true;
			object = (EffectGameObject*) m_gameObjects->objectAtIndex(0);
			if (object == nullptr) return true;
		}

		if (m_offsetX != -99999 && m_offsetXInput != nullptr)
			m_offsetXInput->setString(gd::string(fmt::format("{}", object->m_moveOffset.x / 3.0f)));
		if (m_offsetY != -99999 && m_offsetYInput != nullptr)
			m_offsetYInput->setString(gd::string(fmt::format("{}", object->m_moveOffset.y / 3.0f)));
		if (m_moveTimeInput != nullptr)
			m_moveTimeInput->setString(gd::string(fmt::format("{}", m_moveTime)));
		return true;
	}

	void textChanged(CCTextInputNode* inputNode) override {
		if (!precisionParams) return SetupCameraOffsetTrigger::textChanged(inputNode);
		if (m_disableTextDelegate) return;

		gd::string str = inputNode->getString();
		//ideally we would use something more robust like strtof or stof instead,
		//but this is what the original code does and we are just trying to replicate that
		float value = (float) std::atof(str.c_str());

		int type = inputNode->getTag();
		float sliderValue;
		Slider* slider;
		switch (type) {
			case 0: //X position
				updateTriggers(this, [value](EffectGameObject* object) {
					//yes, I checked the decompiled code, and the original function
					//specifically does convert to a double, multiply, then convert back to a float
					object->m_moveOffset.x = float(value * 3.0);
				});
				sliderValue = std::clamp(float(value / 200.0 + 0.5), 0.0f, 1.0f);
				slider = m_offsetXSlider;
				break;
			case 1: //Y position
				updateTriggers(this, [value](EffectGameObject* object) {
					object->m_moveOffset.y = float(value * 3.0);
				});
				sliderValue = std::clamp(float(value / 200.0 + 0.5), 0.0f, 1.0f);
				slider = m_offsetYSlider;
				break;
			default: //move time
				m_moveTime = value;
				updateTriggers(this, [value](EffectGameObject* object) {
					object->m_duration = value;
				});
				sliderValue = std::clamp(float(value / 10.0), 0.0f, 1.0f);
				slider = m_moveTimeSlider;
				break;
		}
		slider->setValue(sliderValue);
	}
};
//side note: this game's code has horrible naming schemes
#include <Geode/modify/GJFollowCommandLayer.hpp>
class $modify(PrecisionFollowCommandLayer, GJFollowCommandLayer) {
	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("GJFollowCommandLayer::textChanged", Priority::Early)) {
			log::error("failed to set hook priority for GJFollowCommandLayer::textChanged");
		}
	}

	bool init(EffectGameObject* p0, CCArray* p1) {
		if (!GJFollowCommandLayer::init(p0, p1)) return false;
		if (!precisionParams) return true;

		if (m_xModInput != nullptr)
			m_xModInput->setString(gd::string(fmt::format("{}", m_xMod)));
		if (m_yModInput != nullptr)
			m_yModInput->setString(gd::string(fmt::format("{}", m_yMod)));
		if (m_moveTimeInput != nullptr)
			m_moveTimeInput->setString(gd::string(fmt::format("{}", m_moveTime)));
		return true;
	}

	void textChanged(CCTextInputNode* inputNode) override {
		if (!precisionParams) return GJFollowCommandLayer::textChanged(inputNode);
		if (m_disableTextDelegate) return;

		gd::string str = inputNode->getString();

		int type = inputNode->getTag();
		float sliderValue = 0.0;
		Slider* slider = nullptr;
		switch (type) {
			case 0: { //X modifier
				float value = (float) std::atof(str.c_str());
				m_xMod = value;
				updateXMod();
				sliderValue = std::clamp(float(value / 2.0 + 0.5), 0.0f, 1.0f);
				slider = m_xModSlider;
				break;
			}
			case 1: { //Y modifier
				float value = (float) std::atof(str.c_str());
				m_yMod = value;
				updateYMod();
				sliderValue = std::clamp(float(value / 2.0 + 0.5), 0.0f, 1.0f);
				slider = m_yModSlider;
				break;
			}
			case 2: { //target group
				int value = std::atoi(str.c_str());
				m_targetGroupID = std::max(0, value);
				updateTargetGroupID();
				updateTriggers(this, [](EffectGameObject* object) {
					LevelEditorLayer::updateObjectLabel(object);
				});
				break;
			}
			case 4: { //follow group
				int value = std::atoi(str.c_str());
				m_followGroupID = std::max(0, value);
				updateTargetGroupID();
				break;
			}
			default: { //duration (3)
				float value = (float) std::atof(str.c_str());
				m_moveTime = value;
				updateTriggers(this, [value](EffectGameObject* object) {
					object->m_duration = value;
				});
				sliderValue = std::clamp(float(value / 10.0), 0.0f, 1.0f);
				slider = m_moveTimeSlider;
				break;
			}
		}
		if (slider != nullptr)
			slider->setValue(sliderValue);
	}
};
#include <Geode/modify/ColorSelectPopup.hpp>
class $modify(PrecisionColorSelect, ColorSelectPopup) {
	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("ColorSelectPopup::updateOpacityLabel", Priority::Early)) {
			log::error("failed to set hook priority forColorSelectPopup::updateOpacityLabel");
		}
	}

	struct Fields {
		CCMenu* opacityMenu;
		CCLabelBMFont* newOpacityLabel;
		TextInput* opacityField;
	};

	bool init(EffectGameObject* p0, CCArray* p1, ColorAction* p2) {
		if (!ColorSelectPopup::init(p0, p1, p2)) return false;

		if (precisionParams && m_fadeTimeInput != nullptr)
			m_fadeTimeInput->setString(gd::string(fmt::format("{}", m_fadeTime)));

		if (!sliderInputs) return true;

		m_opacityLabel->setOpacity(0);
		m_fields->opacityMenu = new CCMenu();
		m_fields->opacityMenu->setPosition(m_opacityLabel->getPosition());
		m_fields->opacityMenu->setLayout(RowLayout::create()
										 ->setAutoScale(false)
										 ->setAutoGrowAxis(true)
										 ->setCrossAxisOverflow(true));
		m_fields->opacityMenu->setID("hpe-opacity-container");
		m_mainLayer->addChild(m_fields->opacityMenu);

		m_fields->newOpacityLabel = CCLabelBMFont::create("Opacity:", "goldFont.fnt");
		m_fields->newOpacityLabel->setScale(0.65);
		m_fields->newOpacityLabel->setID("hpe-opacity-label");
		m_fields->opacityMenu->addChild(m_fields->newOpacityLabel);

		m_fields->opacityField = TextInput::create(40, "");
		m_fields->opacityField->setID("hpe-opacity-field");
		m_fields->opacityField->setCommonFilter(CommonFilter::Float);
		m_fields->opacityField->setString(gd::string(fmt::format("{}", m_opacity)));
		m_fields->opacityField->setCallback([this](const gd::string& str) {
			try {
				m_opacity = std::stof(str);
				updateOpacity();
			} catch (std::invalid_argument&) {}
			  catch (std::out_of_range&) {}
		});
		m_fields->opacityMenu->addChild(m_fields->opacityField);

		m_fields->opacityMenu->updateLayout();

		return true;
	}
	void updateOpacityLabel() {
		if (!precisionParams) return ColorSelectPopup::updateOpacityLabel();

		if (m_opacityLabel == nullptr || m_fields->opacityField == nullptr || m_fields->opacityField->getInputNode() == nullptr) return;
		m_fields->opacityField->setString(gd::string(fmt::format("{}", m_opacity)));
	}

	void textChanged(CCTextInputNode* inputNode) override {
		//i am too lazy to rewrite all that logic
		ColorSelectPopup::textChanged(inputNode);
		if (!precisionParams) return;
		if (m_disableTextDelegate) return;

		int type = inputNode->getTag();
		if (type == 5) { //fade time
			float value = (float) std::atof(inputNode->getString().c_str());
			m_fadeTime = value;
			updateTriggers(this, [value](EffectGameObject* object) {
				object->m_duration = value;
			});
			m_fadeTimeSlider->setValue(std::clamp(float(value / 10.0), 0.0f, 1.0f));
		}
	}
};
#include <Geode/modify/SetupPulsePopup.hpp>
class $modify(PrecisionPulsePopup, SetupPulsePopup) {
	bool init(EffectGameObject* p0, CCArray* p1) {
		if (!SetupPulsePopup::init(p0, p1)) return false;
		if (!precisionParams) return true;

		if (m_fadeInInput != nullptr)
			m_fadeInInput->setString(gd::string(fmt::format("{}", m_fadeInTime)));
		if (m_holdInput != nullptr)
			m_holdInput->setString(gd::string(fmt::format("{}", m_holdTime)));
		if (m_fadeOutInput != nullptr)
			m_fadeOutInput->setString(gd::string(fmt::format("{}", m_fadeOutTime)));

		return true;
	}

	void textChanged(CCTextInputNode* inputNode) override {
		//i am still too lazy to rewrite all that logic
		SetupPulsePopup::textChanged(inputNode);
		if (!precisionParams) return;
		if (m_disableTextDelegate) return;

		int type = inputNode->getTag();
		switch (type) {
			case 8: { //fade in time
				float value = (float) std::atof(inputNode->getString().c_str());
				m_fadeInTime = value;
				updateTriggers(this, [value](EffectGameObject* object) {
					object->m_fadeInDuration = value;
				});
				m_fadeInSlider->setValue(std::clamp(float(value / 10.0), 0.0f, 1.0f));
			}
			case 9: { //hold time
				float value = (float) std::atof(inputNode->getString().c_str());
				m_holdTime = value;
				updateTriggers(this, [value](EffectGameObject* object) {
					object->m_holdDuration = value;
				});
				m_holdSlider->setValue(std::clamp(float(value / 10.0), 0.0f, 1.0f));
			}
			case 10: { //fade out time
				float value = (float) std::atof(inputNode->getString().c_str());
				m_fadeOutTime = value;
				updateTriggers(this, [value](EffectGameObject* object) {
					object->m_fadeOutDuration = value;
				});
				m_holdSlider->setValue(std::clamp(float(value / 10.0), 0.0f, 1.0f));
			}
			default: break;
		}
	}
};
#include <Geode/modify/SetupOpacityPopup.hpp>
class $modify(PrecisionOpacityPopup, SetupOpacityPopup) {
	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("SetupOpacityPopup::textChanged", Priority::Early)) {
			log::error("failed to set hook priority for SetupOpacityPopup::textChanged");
		}
		if (!self.setHookPriorityPost("SetupOpacityPopup::updateOpacityLabel", Priority::Early)) {
			log::error("failed to set hook priority for SetupOpacityPopup::updateOpacityLabel");
		}
	}

	struct Fields {
		CCLabelBMFont* newOpacityLabel;
		TextInput* opacityField;
	};

	bool init(EffectGameObject* p0, CCArray* p1) {
		if (!SetupOpacityPopup::init(p0, p1)) return false;

		if (precisionParams && m_fadeTimeInput != nullptr)
			m_fadeTimeInput->setString(gd::string(fmt::format("{}", m_fadeTime)));

		if (!sliderInputs) return true;

		m_opacityLabel->setOpacity(0);

		m_fields->newOpacityLabel = CCLabelBMFont::create("Opacity: ", "goldFont.fnt");
		m_fields->newOpacityLabel->setScale(0.7);
		m_fields->newOpacityLabel->setPosition(m_opacityLabel->getPosition());
		m_fields->newOpacityLabel->setAnchorPoint(ccp(1.0f, 0.5f));
		m_fields->newOpacityLabel->setID("hpe-opacity-label");
		m_mainLayer->addChild(m_fields->newOpacityLabel);

		m_fields->opacityField = TextInput::create(70, "");
		m_fields->opacityField->setID("hpe-opacity-field");
		m_fields->opacityField->setPosition(m_fadeTimeInput->getPositionX(), m_opacityLabel->getPositionY());
		m_fields->opacityField->setCommonFilter(CommonFilter::Float);
		if (precisionParams)
			m_fields->opacityField->setString(gd::string(fmt::format("{}", m_opacity)));
		else
			m_fields->opacityField->setString(gd::string(fmt::format("{:.2f}", m_fadeTime)));
		m_fields->opacityField->setCallback([this](const gd::string& str) {
			try {
				m_opacity = std::stof(str);
				updateTriggers(this, [str, this](EffectGameObject* object) {
					object->m_opacity = m_opacity;
				});
				m_opacitySlider->setValue(std::clamp(m_opacity, 0.0f, 1.0f));
			} catch (std::invalid_argument&) {}
			  catch (std::out_of_range&) {}
		});
		m_mainLayer->addChild(m_fields->opacityField);

		return true;
	}
	void updateOpacityLabel() {
		if (!precisionParams) return SetupOpacityPopup::updateOpacityLabel();
		if (m_opacityLabel == nullptr || m_fields->opacityField == nullptr || m_fields->opacityField->getInputNode() == nullptr) return;
		m_fields->opacityField->setString(gd::string(fmt::format("{}", m_opacity)));
	}

	void textChanged(CCTextInputNode* inputNode) override {
		if (!precisionParams) return SetupOpacityPopup::textChanged(inputNode);
		if (m_disableTextDelegate) return;

		gd::string str = inputNode->getString();

		int type = inputNode->getTag();
		if (type == 3) { //target group ID
			m_groupID = std::atoi(str.c_str());
			updateTargetID();
			updateTriggers(this, [](EffectGameObject* object) {
				LevelEditorLayer::updateObjectLabel(object);
			});
		} else { //fade time
			m_fadeTime = (float) std::atof(str.c_str());
			updateDuration();
			m_fadeTimeSlider->setValue(float(m_fadeTime / 10.0));
		}
	}
};
#include <Geode/modify/SetupRandTriggerPopup.hpp>
class $modify(PrecisionRandTriggerPopup, SetupRandTriggerPopup) {
	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("SetupRandTriggerPopup::textChanged", Priority::Early)) {
			log::error("failed to set hook priority for SetupRandTriggerPopup::textChanged");
		}
	}

	void textChanged(CCTextInputNode* inputNode) override {
		if (!precisionParams) return SetupRandTriggerPopup::textChanged(inputNode);
		if (m_disableTextDelegate) return;

		gd::string str = inputNode->getString();

		int type = inputNode->getTag();
		switch (type) {
			case 3: { //target id 1
				int value = std::atoi(str.c_str());
				m_groupID1 = value;
				updateTargetID();
				updateTriggers(this, [](EffectGameObject* object) {
					LevelEditorLayer::updateObjectLabel(object);
				});
			}
			case 5: { //target id 2
				int value = std::atoi(str.c_str());
				m_groupID2 = value;
				updateTargetID2();
				updateTriggers(this, [](EffectGameObject* object) {
					LevelEditorLayer::updateObjectLabel(object);
				});
			}
			case 4: { //chance
				float value = (float) std::atof(str.c_str());
				m_chancePercent = value;
				updateTriggers(this, [value, this](EffectGameObject* object) {
					object->m_duration = value;
				});
			}
			default: break;
		}
	}
};