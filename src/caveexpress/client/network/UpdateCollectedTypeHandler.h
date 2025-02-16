#pragma once

#include "network/IProtocolHandler.h"
#include "caveexpress/shared/network/messages/UpdateCollectedTypeMessage.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "client/ClientMap.h"
#include "ui/UI.h"
#include "caveexpress/client/ui/windows/UIMapWindow.h"

namespace caveexpress {

class UpdateCollectedTypeHandler: public ClientProtocolHandler<UpdateCollectedTypeMessage> {
private:
	ClientMap& _map;
public:
	UpdateCollectedTypeHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const UpdateCollectedTypeMessage* msg) override
	{
		const EntityType& type = msg->getEntityType();
		const bool collected = msg->isCollected();
		ClientPlayer* player = _map.getPlayer();
		if (player != nullptr) {
			if (collected)
				player->setCollected(type);
			else
				player->setCollected(EntityType::NONE);
		}

		UINodeSprite* node = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_COLLECTED);
		if (!collected || type.isNone()) {
			if (node)
				node->clearSprites();
			return;
		}

		if (node) {
			const Animation& animation = EntityTypes::hasDirection(type) ? Animations::ANIMATION_IDLE_RIGHT : Animations::ANIMATION_IDLE;
			const std::string name = SpriteDefinition::get().getSpriteName(type, animation);
			const SpritePtr& sprite = UI::get().loadSprite(name);
			node->addSprite(sprite);
			node->flash();
		}
		if (!_map.wantInformation(type))
			return;

		UINode* mapNode = UI::get().getNode<UINode>(UI_WINDOW_MAP, UINODE_MAP);
		if (!mapNode)
			return;
		if (EntityTypes::isStone(type)) {
			mapNode->displayText(tr("Drop the stone to collect packages again"));
			if (System.hasTouch())
				mapNode->displayText(tr("Use the second finger to drop the stone"));
		} else if (EntityTypes::isPackage(type)) {
			mapNode->displayText(tr("Drop off at the shredder"));
			if (System.hasTouch())
				mapNode->displayText(tr("Use the second finger to drop the package"));
		} else if (EntityTypes::isNpcGrandpa(type)) {
			mapNode->displayText(tr("Transfer the grandpa to the desired cave"));
		} else if (EntityTypes::isNpcMan(type)) {
			mapNode->displayText(tr("Transfer the man to the desired cave"));
		} else if (EntityTypes::isNpcWoman(type)) {
			mapNode->displayText(tr("Transfer the woman to the desired cave"));
		}
 	}
};

}
