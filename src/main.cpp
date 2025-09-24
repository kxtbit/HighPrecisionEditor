// ReSharper disable CppHidingFunction
#include <Geode/Geode.hpp>

using namespace geode::prelude;

std::string patchSaveString(std::string save, CCObject* self, std::string (*patcher)(CCObject*, const int, std::string)) {
	size_t pos = 0;
	std::stringstream out;
	char c;
	bool first = true;
	while (pos < save.size()) {
		if (first) {
			first = false;
		} else {
			out << ',';
		}
		size_t key_start = pos;
		do {
			c = save[pos++];
			if (pos >= save.size()) {
				log::warn("Object string ended prematurely, will abort processing of this save string");
				return save;
			}
		} while (c != ',');

		std::string keyStr = std::string(
			save.data() + key_start,
			std::min(pos - key_start - 1, save.size() - key_start));
		auto keyResult = utils::numFromString<int>(keyStr);
		if (!keyResult.isOk()) {
			log::warn("Failed to parse object string key {}, will abort processing of this save string", keyStr);
			return save;
		}
		const int key = keyResult.unwrap();
		out << key << ',';

		size_t val_start = pos;
		do {
			c = save[pos++];
			if (pos >= save.size()) {
				pos++;
				break;
			}
		} while (c != ',');

		out << patcher(self, key, std::string(save.data() + val_start, std::min(pos - val_start - 1, save.size() - val_start)));
	}

	return out.str();
}

bool precisionPosition = true;
bool precisionRotation = true;
bool precisionScale = true;
bool precisionParams = true;
bool decimalMoveParams = true;
bool sliderInputs = true;
bool miscUIFixes = true;

$execute {
	precisionPosition = Mod::get()->getSettingValue<bool>("full-precision-object-position");
	precisionRotation = Mod::get()->getSettingValue<bool>("full-precision-object-rotation");
	precisionScale = Mod::get()->getSettingValue<bool>("full-precision-object-scale");
	precisionParams = Mod::get()->getSettingValue<bool>("full-precision-trigger-parameters");
	decimalMoveParams = Mod::get()->getSettingValue<bool>("allow-decimal-move-parameters");
	sliderInputs = Mod::get()->getSettingValue<bool>("enable-slider-inputs");
	miscUIFixes = Mod::get()->getSettingValue<bool>("misc-ui-fixes");
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
	listenForSettingChanges("misc-ui-fixes", [](bool value) {
		miscUIFixes = value;
	});
}

#include <Geode/modify/GameObject.hpp>
class $modify(PrecisionGameObject, GameObject) {
	gd::string getSaveString(GJBaseGameLayer* layer) override {
		gd::string save = GameObject::getSaveString(layer);
		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionGameObject*) rawSelf;
			switch (key) {
				case 2:
					if (!precisionPosition) return orig;
					return fmt::format("{}", self->m_positionX);
				case 3:
					if (!precisionPosition) return orig;
					return fmt::format("{}", self->m_positionY - 90);
				case 32:
					if (!precisionScale) return orig;
					return fmt::format("{}", std::max(self->m_scaleX, self->m_scaleY));
				case 6:
				case 131:
					if (!precisionRotation) return orig;
					return fmt::format("{}", self->m_fRotationX);
				case 132:
					if (!precisionRotation) return orig;
					return fmt::format("{}", self->m_fRotationY);
				case 128:
					if (!precisionScale) return orig;
					return fmt::format("{}", self->m_scaleX);
				case 129:
					if (!precisionScale) return orig;
					return fmt::format("{}", self->m_scaleY);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionEffectObject*) rawSelf;
			switch (key) {
					//trigger common
				case 10: //also used as random trigger chance
					return fmt::format("{}", self->m_duration);
				case 85:
					return fmt::format("{}", self->m_easingRate);

					//move trigger
				case 28: //also used by camera offset and guide triggers
					return fmt::format("{}", self->m_moveOffset.x);
				case 29: //also used by camera offset and guide triggers
					return fmt::format("{}", self->m_moveOffset.y);
				case 143:
					return fmt::format("{}", self->m_moveModX);
				case 144:
					return fmt::format("{}", self->m_moveModY);

					//rotate trigger
				case 68: //also used by camera rotate trigger
					return fmt::format("{}", self->m_rotationDegrees);
				case 402:
					return fmt::format("{}", self->m_rotationOffset);

					//pulse trigger
				case 45:
					return fmt::format("{}", self->m_fadeInDuration);
				case 46:
					return fmt::format("{}", self->m_holdDuration);
				case 47:
					return fmt::format("{}", self->m_fadeOutDuration);

					//alpha trigger
				case 35:
					return fmt::format("{}", self->m_opacity);

					//shake trigger
				case 75:
					return fmt::format("{}", self->m_shakeStrength);
				case 84:
					return fmt::format("{}", self->m_shakeInterval);

					//follow trigger
				case 72:
					return fmt::format("{}", self->m_followXMod);
				case 73:
					return fmt::format("{}", self->m_followYMod);

					//follow player y trigger
				case 90:
					return fmt::format("{}", self->m_followYSpeed);
				case 91:
					return fmt::format("{}", self->m_followYDelay);
				case 105:
					return fmt::format("{}", self->m_followYMaxSpeed);

					//camera zoom trigger
				case 371: //also used by camera guide
					return fmt::format("{}", self->m_zoomValue);

					//camera mode trigger
				case 114:
					return fmt::format("{}", self->m_cameraPaddingValue);

					//timewarp trigger
				case 120:
					return fmt::format("{}", self->m_timeWarpTimeMod);

					//gravity trigger
				case 148:
					return fmt::format("{}", self->m_gravityValue);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionTransformTrigger*) rawSelf;
			switch (key) {
					//scale trigger
				case 150:
					return fmt::format("{}", self->m_objectScaleX);
				case 151:
					return fmt::format("{}", self->m_objectScaleY);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionKeyframeAnimTrigger*) rawSelf;
			switch (key) {
				case 520:
					return fmt::format("{}", self->m_timeMod);
				case 521:
					return fmt::format("{}", self->m_positionXMod);
				case 545:
					return fmt::format("{}", self->m_positionYMod);
				case 522:
					return fmt::format("{}", self->m_rotationMod);
				case 523:
					return fmt::format("{}", self->m_scaleXMod);
				case 546:
					return fmt::format("{}", self->m_scaleYMod);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionKeyframeGameObject*) rawSelf;
			switch (key) {
				case 557:
					return fmt::format("{}", self->m_spawnDelay);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionGradientTrigger*) rawSelf;
			switch (key) {
				case 456:
					return fmt::format("{}", self->m_previewOpacity);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionCameraTrigger*) rawSelf;
			switch (key) {
				case 213:
					return fmt::format("{}", self->m_followEasing);
				case 454:
					return fmt::format("{}", self->m_velocityModifier);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionItemTrigger*) rawSelf;
			switch (key) {
				case 479:
					return fmt::format("{}", self->m_mod1);
				case 483:
					return fmt::format("{}", self->m_mod2);
				case 484:
					return fmt::format("{}", self->m_tolerance);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionSFXTrigger*) rawSelf;
			switch (key) {
				case 406: //also used by song trigger
					return fmt::format("{}", self->m_volume);
				case 421:
					return fmt::format("{}", self->m_volumeNear);
				case 422:
					return fmt::format("{}", self->m_volumeMedium);
				case 423:
					return fmt::format("{}", self->m_volumeFar);
				case 434:
					return fmt::format("{}", self->m_minInterval);
				case 490:
					return fmt::format("{}", self->m_soundDuration);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionTimerTrigger*) rawSelf;
			switch (key) {
				case 467:
					return fmt::format("{}", self->m_startTime);
				case 473: //also used by time event trigger
					return fmt::format("{}", self->m_targetTime);
				case 470:
					return fmt::format("{}", self->m_timeMod);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionSpawnTrigger*) rawSelf;
			switch (key) {
				case 63:
					return fmt::format("{}", self->m_spawnDelay);
				case 556:
					return fmt::format("{}", self->m_delayRange);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionSequenceTrigger*) rawSelf;
			switch (key) {
				case 437:
					return fmt::format("{}", self->m_minInt);
				case 438:
					return fmt::format("{}", self->m_reset);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionSpawnParticle*) rawSelf;
			switch (key) {
				case 554:
					return fmt::format("{}", self->m_scale);
				case 555:
					return fmt::format("{}", self->m_scaleVariance);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionRotateGameplay*) rawSelf;
			switch (key) {
				case 582:
					return fmt::format("{}", self->m_velocityModX);
				case 583:
					return fmt::format("{}", self->m_velocityModY);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionGameOptions*) rawSelf;
			switch (key) {
				case 574:
					return fmt::format("{}", self->m_respawnTime);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionTeleportPortal*) rawSelf;
			switch (key) {
				case 348:
					return fmt::format("{}", self->m_redirectForceMin);
				case 349:
					return fmt::format("{}", self->m_redirectForceMax);
				case 350:
					return fmt::format("{}", self->m_redirectForceMod);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionShaderGameObject*) rawSelf;
			switch (key) {
				case 175:
					return fmt::format("{}", self->m_speed);
				case 176:
					return fmt::format("{}", self->m_strength);
				case 179:
					return fmt::format("{}", self->m_waveWidth);
				case 180:
					return fmt::format("{}", self->m_targetX);
				case 189:
					return fmt::format("{}", self->m_targetY);
				case 181:
					return fmt::format("{}", self->m_fadeIn);
				case 182:
					return fmt::format("{}", self->m_fadeOut);
				case 177:
					return fmt::format("{}", self->m_timeOff);
				case 512:
					return fmt::format("{}", self->m_maxSize);
				case 290:
					return fmt::format("{}", self->m_screenOffsetX);
				case 291:
					return fmt::format("{}", self->m_screenOffsetY);
				case 183:
					return fmt::format("{}", self->m_inner);
				case 191:
					return fmt::format("{}", self->m_outer);

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

		return patchSaveString(save, this, [](CCObject* rawSelf, const int key, std::string orig) {
			auto self = (PrecisionForceBlock*) rawSelf;
			switch (key) {
				case 149:
					return fmt::format("{}", self->m_force);
				case 526:
					return fmt::format("{}", self->m_minForce);
				case 527:
					return fmt::format("{}", self->m_maxForce);

				default:
					return orig;
			}
		});
	}
};

#include <Geode/modify/SetupTriggerPopup.hpp>
class $modify(PrecisionTriggerPopup, SetupTriggerPopup) {
	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("SetupTriggerPopup::triggerSliderChanged", Priority::Late)) {
			log::error("failed to set hook priority for SetupTriggerPopup::triggerSliderChanged");
		}
		#if defined(GEODE_IS_MACOS) || defined(GEODE_IS_IOS)
		if (!self.setHookPriorityPost("SetupTriggerPopup::textChanged", Priority::Late)) {
			log::error("failed to set hook priority for SetupTriggerPopup::textChanged");
		}
		#endif
	}

	void updateInputNode(int property, float value) override {
		SetupTriggerPopup::updateInputNode(property, value);
		if (!precisionParams) return;

		auto inputNode = (CCTextInputNode*) m_inputNodes->objectForKey(property);
		if (inputNode == nullptr || inputNode->m_textField == nullptr) {
			//log::info("skipped input node due to null text field");
			return;
		}
		const std::string newStr = fmt::format("{}", value);
		const std::string oldStr = inputNode->getString();
		auto oldResult = utils::numFromString<float>(oldStr);
		//log::info("update input node {} -> {}", oldStr, newStr);
		if (!oldResult.isOk() || oldResult.unwrap() != value || newStr.size() < oldStr.size()) {
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
	#ifndef GEODE_IS_IOS
	float getTruncatedValue(float value, int decimalPlaces) {
		if (!precisionParams) return SetupTriggerPopup::getTruncatedValue(value, std::abs(decimalPlaces));

		if (decimalPlaces != 0) {
			//log::info("overrode truncation of {} from {} places", value, decimalPlaces);
			return value;
		}
		return SetupTriggerPopup::getTruncatedValue(value, std::abs(decimalPlaces));
	}
	#endif

	#if defined(GEODE_IS_MACOS) || defined(GEODE_IS_IOS)
	void textChanged(CCTextInputNode* inputNode) override {
		if (!precisionParams) return SetupTriggerPopup::textChanged(inputNode);

		if (m_disableTextDelegate) return;

		int property = inputNode->getTag();
		std::string str = inputNode->getString();
		float value = utils::numFromString<float>(str).unwrapOr(0);

		updateInputValue(property, value);
		m_triggerValues->setObject(CCFloat::create(value), property);
		valueChanged(property, value);
		updateSlider(property, triggerSliderValueFromValue(property, value));
	}
	#endif

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
		if (!self.setHookPriorityPost("SetupCameraOffsetTrigger::textChanged", Priority::Late)) {
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
			m_offsetXInput->setString(fmt::format("{}", object->m_moveOffset.x / 3.0f));
		if (m_offsetY != -99999 && m_offsetYInput != nullptr)
			m_offsetYInput->setString(fmt::format("{}", object->m_moveOffset.y / 3.0f));
		if (m_moveTimeInput != nullptr)
			m_moveTimeInput->setString(fmt::format("{}", m_moveTime));
		return true;
	}

	void textChanged(CCTextInputNode* inputNode) override {
		if (!precisionParams) return SetupCameraOffsetTrigger::textChanged(inputNode);
		if (m_disableTextDelegate) return;

		std::string str = inputNode->getString();
		float value = utils::numFromString<float>(str).unwrapOr(0);

		int type = inputNode->getTag();
		float sliderValue;
		Slider* slider;
		switch (type) {
			case 0: //X position
				m_offsetX = value * 3.0;
				updateMoveCommandPosX();
				sliderValue = std::clamp(float(value / 200.0 + 0.5), 0.0f, 1.0f);
				slider = m_offsetXSlider;
				break;
			case 1: //Y position
				m_offsetY = value * 3.0;
				updateMoveCommandPosY();
				sliderValue = std::clamp(float(value / 200.0 + 0.5), 0.0f, 1.0f);
				slider = m_offsetYSlider;
				break;
			default: //move time
				m_moveTime = value;
				updateDuration();
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
		if (!self.setHookPriorityPost("GJFollowCommandLayer::textChanged", Priority::Late)) {
			log::error("failed to set hook priority for GJFollowCommandLayer::textChanged");
		}
	}

	bool init(EffectGameObject* p0, CCArray* p1) {
		if (!GJFollowCommandLayer::init(p0, p1)) return false;
		if (!precisionParams) return true;

		if (m_xModInput != nullptr)
			m_xModInput->setString(fmt::format("{}", m_xMod));
		if (m_yModInput != nullptr)
			m_yModInput->setString(fmt::format("{}", m_yMod));
		if (m_moveTimeInput != nullptr)
			m_moveTimeInput->setString(fmt::format("{}", m_moveTime));
		return true;
	}

	void textChanged(CCTextInputNode* inputNode) override {
		if (!precisionParams) return GJFollowCommandLayer::textChanged(inputNode);
		if (m_disableTextDelegate) return;

		std::string str = inputNode->getString();

		int type = inputNode->getTag();
		float sliderValue = 0.0;
		Slider* slider = nullptr;
		switch (type) {
			case 0: { //X modifier
				float value = utils::numFromString<float>(str).unwrapOr(0);
				m_xMod = value;
				updateXMod();
				sliderValue = std::clamp(float(value / 2.0 + 0.5), 0.0f, 1.0f);
				slider = m_xModSlider;
				break;
			}
			case 1: { //Y modifier
				float value = utils::numFromString<float>(str).unwrapOr(0);
				m_yMod = value;
				updateYMod();
				sliderValue = std::clamp(float(value / 2.0 + 0.5), 0.0f, 1.0f);
				slider = m_yModSlider;
				break;
			}
			case 2: { //target group
				int value = utils::numFromString<int>(str).unwrapOr(0);
				m_targetGroupID = std::max(0, value);
				updateTargetGroupID();
				updateEditorLabel();
				break;
			}
			case 4: { //follow group
				int value = utils::numFromString<int>(str).unwrapOr(0);
				m_followGroupID = std::max(0, value);
				updateTargetGroupID2();
				break;
			}
			default: { //duration (3)
				float value = utils::numFromString<float>(str).unwrapOr(0);
				m_moveTime = value;
				updateDuration();
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
		if (!self.setHookPriorityPost("ColorSelectPopup::updateOpacityLabel", Priority::Late)) {
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
			m_fadeTimeInput->setString(fmt::format("{}", m_fadeTime));

		if (!sliderInputs) return true;

		if (m_opacityLabel == nullptr) return true;

		m_opacityLabel->setOpacity(0);
		m_fields->opacityMenu = new CCMenu();
		m_fields->opacityMenu->setPosition(m_opacityLabel->getPosition());
		m_fields->opacityMenu->setLayout(RowLayout::create()
										 ->setAutoScale(false)
										 ->setAutoGrowAxis(true)
										 ->setCrossAxisOverflow(true));
		m_fields->opacityMenu->setID("opacity-container"_spr);
		m_mainLayer->addChild(m_fields->opacityMenu);

		m_fields->newOpacityLabel = CCLabelBMFont::create("Opacity:", "goldFont.fnt");
		m_fields->newOpacityLabel->setScale(0.65);
		m_fields->newOpacityLabel->setID("opacity-label"_spr);
		m_fields->opacityMenu->addChild(m_fields->newOpacityLabel);

		m_fields->opacityField = TextInput::create(50, "");
		m_fields->opacityField->setScale(0.8);
		m_fields->opacityField->setID("opacity-field"_spr);
		m_fields->opacityField->setCommonFilter(CommonFilter::Float);
		m_fields->opacityField->setString(precisionParams ? fmt::format("{}", m_opacity) : fmt::format("{:.2f}", m_opacity));
		m_fields->opacityField->setCallback([this](const std::string& str) {
			auto result = utils::numFromString<float>(str);
			if (!result.isOk()) return;
			m_opacity = result.unwrap();
			updateOpacity();
		});
		m_fields->opacityMenu->addChild(m_fields->opacityField);

		m_fields->opacityMenu->updateLayout();

		return true;
	}
	void updateOpacityLabel() {
		if (!precisionParams && !sliderInputs) return ColorSelectPopup::updateOpacityLabel();

		if (m_opacityLabel == nullptr || m_fields->opacityField == nullptr || m_fields->opacityField->getInputNode() == nullptr) return;
		if (!sliderInputs) {
			m_opacityLabel->setString(fmt::format("Opacity: {}", m_opacity).c_str());
			return;
		}
		m_fields->opacityField->setString(precisionParams ? fmt::format("{}", m_opacity) : fmt::format("{:.2f}", m_opacity));
	}

	#ifdef GEODE_IS_MACOS
	void onDefault(CCObject* sender) {
		ColorSelectPopup::onDefault(sender);

		if ((!precisionParams && !sliderInputs) || !m_gameObject) return;
		if (m_opacityLabel == nullptr || m_fields->opacityField == nullptr || m_fields->opacityField->getInputNode() == nullptr) return;

		if (!sliderInputs) {
			m_opacityLabel->setString(fmt::format("Opacity: {}", m_opacity).c_str());
			return;
		}
		m_fields->opacityField->setString(precisionParams ? fmt::format("{}", m_opacity) : fmt::format("{:.2f}", m_opacity));
	}

	void sliderChanged(CCObject* sender) {
		ColorSelectPopup::sliderChanged(sender);

		if ((!precisionParams && !sliderInputs) || sender->getTag() != 2) return;
		if (m_opacityLabel == nullptr || m_fields->opacityField == nullptr || m_fields->opacityField->getInputNode() == nullptr) return;

		if (!sliderInputs) {
			m_opacityLabel->setString(fmt::format("Opacity: {}", m_opacity).c_str());
			return;
		}
		m_fields->opacityField->setString(precisionParams ? fmt::format("{}", m_opacity) : fmt::format("{:.2f}", m_opacity));
	}
	#endif

	void textChanged(CCTextInputNode* inputNode) override {
		//i am too lazy to rewrite all that logic
		ColorSelectPopup::textChanged(inputNode);
		if (!precisionParams) return;
		if (m_disableTextDelegate) return;

		int type = inputNode->getTag();
		if (type == 5) { //fade time
			float value = utils::numFromString<float>(inputNode->getString()).unwrapOr(0);
			m_fadeTime = value;
			updateDuration();
			m_fadeTimeSlider->setValue(std::clamp(float(value / 10.0), 0.0f, 1.0f));
		}
	}
};
#include <Geode/modify/SetupPulsePopup.hpp>
class $modify(PrecisionPulsePopup, SetupPulsePopup) {
	bool init(EffectGameObject* p0, CCArray* p1) {
		if (!SetupPulsePopup::init(p0, p1)) return false;
		if (!precisionParams) return true;

		bool oldDisableTextDelegate = m_disableTextDelegate;
		m_disableTextDelegate = true;

		if (m_fadeInInput != nullptr)
			m_fadeInInput->setString(fmt::format("{}", m_fadeInTime));
		if (m_holdInput != nullptr)
			m_holdInput->setString(fmt::format("{}", m_holdTime));
		if (m_fadeOutInput != nullptr)
			m_fadeOutInput->setString(fmt::format("{}", m_fadeOutTime));

		m_disableTextDelegate = oldDisableTextDelegate;

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
				float value = utils::numFromString<float>(inputNode->getString()).unwrapOr(0);
				m_fadeInTime = value;
				updateFadeInTime();
				m_fadeInSlider->setValue(std::clamp(float(value / 10.0), 0.0f, 1.0f));
				break;
			}
			case 9: { //hold time
				float value = utils::numFromString<float>(inputNode->getString()).unwrapOr(0);
				m_holdTime = value;
				updateHoldTime();
				m_holdSlider->setValue(std::clamp(float(value / 10.0), 0.0f, 1.0f));
				break;
			}
			case 10: { //fade out time
				float value = utils::numFromString<float>(inputNode->getString()).unwrapOr(0);
				m_fadeOutTime = value;
				updateFadeOutTime();
				m_fadeOutSlider->setValue(std::clamp(float(value / 10.0), 0.0f, 1.0f));
				break;
			}
			default: break;
		}
	}
};
#include <Geode/modify/SetupOpacityPopup.hpp>
class $modify(PrecisionOpacityPopup, SetupOpacityPopup) {
	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("SetupOpacityPopup::textChanged", Priority::Late)) {
			log::error("failed to set hook priority for SetupOpacityPopup::textChanged");
		}
		if (!self.setHookPriorityPost("SetupOpacityPopup::updateOpacityLabel", Priority::Late)) {
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
			m_fadeTimeInput->setString(fmt::format("{}", m_fadeTime));

		if (miscUIFixes && m_fadeTimeSlider != nullptr
				&& m_fadeTimeSlider->m_groove != nullptr
				&& m_fadeTimeSlider->m_touchLogic != nullptr
				&& m_fadeTimeSlider->m_touchLogic->m_thumb != nullptr) {
			//make the fade time slider the same size as the opacity slider for consistency

			float sliderValue = m_fadeTimeSlider->getValue();
			m_fadeTimeSlider->m_groove->setScale(1.0);
			m_fadeTimeSlider->m_touchLogic->m_length = 200.0;
			m_fadeTimeSlider->m_touchLogic->m_thumb->setScale(1.0);
			m_fadeTimeSlider->setValue(sliderValue);
		}

		if (!sliderInputs) return true;

		m_opacityLabel->setOpacity(0);

		m_fields->newOpacityLabel = CCLabelBMFont::create("Opacity: ", "goldFont.fnt");
		m_fields->newOpacityLabel->setScale(0.7);
		m_fields->newOpacityLabel->setPosition(m_opacityLabel->getPosition());
		m_fields->newOpacityLabel->setAnchorPoint(ccp(1.0f, 0.5f));
		m_fields->newOpacityLabel->setID("opacity-label"_spr);
		m_mainLayer->addChild(m_fields->newOpacityLabel);

		m_fields->opacityField = TextInput::create(70, "");
		m_fields->opacityField->setID("opacity-field"_spr);
		m_fields->opacityField->setZOrder(-1); //make it show behind the slider
		m_fields->opacityField->setPosition(m_fadeTimeInput->getPositionX(), m_opacityLabel->getPositionY());
		m_fields->opacityField->setCommonFilter(CommonFilter::Float);
		m_fields->opacityField->setString(precisionParams ? fmt::format("{}", m_opacity) : fmt::format("{:.2f}", m_opacity));
		m_fields->opacityField->setCallback([this](const std::string& str) {
			auto result = utils::numFromString<float>(str);
			if (!result.isOk()) return;
			m_opacity = result.unwrap();
			updateOpacity();
			m_opacitySlider->setValue(std::clamp(m_opacity, 0.0f, 1.0f));
		});
		m_mainLayer->addChild(m_fields->opacityField);

		return true;
	}
	void updateOpacityLabel() {
		if (!precisionParams && !sliderInputs) return SetupOpacityPopup::updateOpacityLabel();
		if (m_opacityLabel == nullptr || m_fields->opacityField == nullptr || m_fields->opacityField->getInputNode() == nullptr) return;

		if (!sliderInputs) {
			m_opacityLabel->setString(fmt::format("Opacity: {}", m_opacity).c_str());
			return;
		}
		m_fields->opacityField->setString(precisionParams ? fmt::format("{}", m_opacity) : fmt::format("{:.2f}", m_opacity));
	}

	#ifdef GEODE_IS_MACOS
	void sliderChanged(CCObject* sender) {
		SetupOpacityPopup::sliderChanged(sender);

		if ((!precisionParams && !sliderInputs) || sender->getTag() != 2) return;
		if (m_opacityLabel == nullptr) return;

		if (!sliderInputs) {
			m_opacityLabel->setString(fmt::format("Opacity: {}", m_opacity).c_str());
			return;
		}
		if (m_fields->opacityField == nullptr || m_fields->opacityField->getInputNode() == nullptr) return;
		m_fields->opacityField->setString(precisionParams ? fmt::format("{}", m_opacity) : fmt::format("{:.2f}", m_opacity));
	}
	#endif

	void textChanged(CCTextInputNode* inputNode) override {
		if (!precisionParams) return SetupOpacityPopup::textChanged(inputNode);
		if (m_disableTextDelegate) return;

		std::string str = inputNode->getString();

		int type = inputNode->getTag();
		if (type == 3) { //target group ID
			m_groupID = utils::numFromString<int>(str).unwrapOr(0);
			updateTargetID();
			updateEditorLabel();
		} else { //fade time
			m_fadeTime = utils::numFromString<float>(str).unwrapOr(0);
			updateDuration();
			m_fadeTimeSlider->setValue(float(m_fadeTime / 10.0));
		}
	}
};
#include <Geode/modify/SetupTimeWarpPopup.hpp>
class $modify(PrecisionTimeWarpPopup, SetupTimeWarpPopup) {
	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("SetupOpacityPopup::updateTimeWarpLabel", Priority::Late)) {
			log::error("failed to set hook priority for SetupOpacityPopup::updateTimeWarpLabel");
		}
	}

	struct Fields {
		CCLabelBMFont* newTimeWarpLabel;
		TextInput* timeWarpField;
	};

	bool init(EffectGameObject* p0, CCArray* p1) {
		if (!SetupTimeWarpPopup::init(p0, p1)) return false;

		if (!sliderInputs) return true;

		m_timeWarpLabel->setOpacity(0);

		m_fields->newTimeWarpLabel = CCLabelBMFont::create("TimeMod: ", "goldFont.fnt");
		m_fields->newTimeWarpLabel->setScale(0.7);
		m_fields->newTimeWarpLabel->setPosition(m_timeWarpLabel->getPosition());
		m_fields->newTimeWarpLabel->setAnchorPoint(ccp(1.0f, 0.5f));
		m_fields->newTimeWarpLabel->setID("time-mod-label"_spr);
		m_mainLayer->addChild(m_fields->newTimeWarpLabel);

		m_fields->timeWarpField = TextInput::create(70, "");
		m_fields->timeWarpField->setID("time-mod-field"_spr);
		m_fields->timeWarpField->setPosition(m_timeWarpLabel->getPositionX() + 40, m_timeWarpLabel->getPositionY());
		m_fields->timeWarpField->setCommonFilter(CommonFilter::Float);
		m_fields->timeWarpField->setString(precisionParams ? fmt::format("{}", m_timeWarpMod) : fmt::format("{:.2f}", m_timeWarpMod));
		m_fields->timeWarpField->setCallback([this](const std::string& str) {
			auto result = utils::numFromString<float>(str);
			if (!result.isOk()) return;
			auto value = result.unwrap();
			m_timeWarpMod = value;
			updateTriggers(this, [value](GameObject* object) {
				((EffectGameObject*) object)->m_timeWarpTimeMod = value;
			});
			m_timeWarpSlider->setValue(std::clamp((value - 0.1f) / 1.9f, 0.0f, 1.0f));
		});
		m_mainLayer->addChild(m_fields->timeWarpField);

		return true;
	};

	#ifdef GEODE_IS_MACOS
	void sliderChanged(CCObject* sender) {
		SetupTimeWarpPopup::sliderChanged(sender);

		if (!precisionParams && !sliderInputs) return;
		if (m_timeWarpLabel == nullptr || m_fields->timeWarpField == nullptr || m_fields->timeWarpField->getInputNode() == nullptr) return;

		if (!sliderInputs) {
			m_timeWarpLabel->setString(fmt::format("Opacity: {}", m_timeWarpMod).c_str());
			return;
		}
		m_fields->timeWarpField->setString(precisionParams ? fmt::format("{}", m_timeWarpMod) : fmt::format("{:.2f}", m_timeWarpMod));
	}
	#endif

	void updateTimeWarpLabel() {
		if (!precisionParams && !sliderInputs) return SetupTimeWarpPopup::updateTimeWarpLabel();
		if (m_timeWarpLabel == nullptr || m_fields->timeWarpField == nullptr || m_fields->timeWarpField->getInputNode() == nullptr) return;

		if (!sliderInputs) {
			m_timeWarpLabel->setString(fmt::format("Opacity: {}", m_timeWarpMod).c_str());
			return;
		}
		m_fields->timeWarpField->setString(precisionParams ? fmt::format("{}", m_timeWarpMod) : fmt::format("{:.2f}", m_timeWarpMod));
	}
};
#include <Geode/modify/SetupRandTriggerPopup.hpp>
class $modify(PrecisionRandTriggerPopup, SetupRandTriggerPopup) {
	static void onModify(auto& self) {
		if (!self.setHookPriorityPost("SetupRandTriggerPopup::textChanged", Priority::Late)) {
			log::error("failed to set hook priority for SetupRandTriggerPopup::textChanged");
		}
	}

	void textChanged(CCTextInputNode* inputNode) override {
		if (!precisionParams) return SetupRandTriggerPopup::textChanged(inputNode);
		if (m_disableTextDelegate) return;

		std::string str = inputNode->getString();

		int type = inputNode->getTag();
		switch (type) {
			case 3: { //target id 1
				int value = utils::numFromString<int>(str).unwrapOr(0);
				m_groupID1 = value;
				updateTargetID();
				updateEditorLabel();
				break;
			}
			case 5: { //target id 2
				int value = utils::numFromString<int>(str).unwrapOr(0);
				m_groupID2 = value;
				updateTargetID2();
				updateEditorLabel();
				break;
			}
			case 4: { //chance
				float value = utils::numFromString<float>(str).unwrapOr(0);
				m_chancePercent = value;
				updateChance();
				break;
			}
			default: break;
		}
	}
};
//disabled this part because it was buggy and caused crashes
/*#include <Geode/modify/ConfigureHSVWidget.hpp>
class $modify(PrecisionHSVWidget, ConfigureHSVWidget) {
	bool init(ccHSVValue hsv, bool unused, bool addInputs) {
		//honestly not sure why there isn't at least an option to configure this in game
		//the code is already there, but the numeric input fields are specifically disabled
		//on every instance of this widget except on the Edit Object screen
		if (sliderInputs) addInputs = true;
		return ConfigureHSVWidget::init(hsv, unused, addInputs);
	}
};*/