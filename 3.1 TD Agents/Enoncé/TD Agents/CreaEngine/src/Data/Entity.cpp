#include "stdafx.h"

#include "json\json.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "Core\Entity.h"
#include "Core\Component.h"
#include "AI\Agent.h"

namespace crea
{
	Entity::Entity()
	{
		m_pParent = nullptr;
		m_szName = "Entity";
	}

	Entity::~Entity()
	{
		clear();
	}

	void Entity::addChild(Entity* _pEntity)
	{
		m_pChildren.push_back(_pEntity);
		_pEntity->setParent(this);
	}

	void Entity::removeChild(Entity* _pEntity)
	{
		_pEntity->setParent(nullptr);
		m_pChildren.remove(_pEntity);
	}

	void Entity::addComponent(Component* _pComponent)
	{
		_pComponent->setEntity(this);
		m_pComponents.push_back(_pComponent);
	}

	void Entity::removeComponent(Component* _pComponent)
	{
		_pComponent->setEntity(nullptr);
		auto a = std::remove_if(m_pComponents.begin(), m_pComponents.end(),
			[=](Component* p) { return p == _pComponent; });

		m_pComponents.erase(a);
	}

	bool Entity::init()
	{
		// Components
		for (list<Component*>::iterator it = m_pComponents.begin(); it != m_pComponents.end(); ++it)
		{
			(*it)->init();
		}

		// Children
		for (list<Entity*>::iterator it = m_pChildren.begin(); it != m_pChildren.end(); ++it)
		{
			(*it)->init();
		}

		return true;
	}

	bool Entity::update()
	{
		// Components
		for (list<Component*>::iterator it = m_pComponents.begin(); it != m_pComponents.end(); ++it)
		{
			(*it)->update();
		}

		// Children
		for (list<Entity*>::iterator it = m_pChildren.begin(); it != m_pChildren.end(); ++it)
		{
			(*it)->update();
		}

		return true;
	}

	bool Entity::draw()
	{
		// Components
		for (list<Component*>::iterator it = m_pComponents.begin(); it != m_pComponents.end(); ++it)
		{
			(*it)->draw();
		}

		// Children
		for (list<Entity*>::iterator it = m_pChildren.begin(); it != m_pChildren.end(); ++it)
		{
			(*it)->draw();
		}


		return true;
	}

	void Entity::clear()
	{
		// Children
		for (list<Entity*>::iterator it = m_pChildren.begin(); it != m_pChildren.end(); ++it)
		{
			delete (*it);
		}
		m_pChildren.clear();

		// Components
		for (list<Component*>::iterator it = m_pComponents.begin(); it != m_pComponents.end(); ++it)
		{
			//delete (*it);// CB: the components are destroyed by managers...
		}
		m_pComponents.clear();
	}

	Entity* Entity::getEntity(string& _szName)
	{
		if (hasName(_szName))
		{
			return this;
		}
		else
		{
			// Children
			for (list<Entity*>::iterator it = m_pChildren.begin(); it != m_pChildren.end(); ++it)
			{
				Entity* pEntity = (*it)->getEntity(_szName);
				if (pEntity)
				{
					return pEntity;
				}
			}
		}
		return nullptr;
	}

	Entity* Entity::getEntity(Entity* _pEntity)
	{
		if (_pEntity == this)
		{
			return this;
		}
		else
		{
			// Children
			for (list<Entity*>::iterator it = m_pChildren.begin(); it != m_pChildren.end(); ++it)
			{
				Entity* pEntity = (*it)->getEntity(_pEntity);
				if (pEntity)
				{
					return pEntity;
				}
			}
		}
		return nullptr;
	}

	bool Entity::removeEntity(Entity* _pEntity)
	{
		if (_pEntity->m_pParent != nullptr)
		{
			_pEntity->m_pParent->removeChild(_pEntity);
			return true;
		}
		return false;
	}

	bool Entity::loadFromFileJSON(string& _filename)
	{
		Json::Value root;
		std::ifstream entityStream( DATAAGENTPATH + _filename, std::ifstream::binary);
		if (entityStream.fail())
		{
			cerr << "Can't open Entity file: " << _filename << endl;
			return false;
		}

		// Parse file
		entityStream >> root;

		crea::GameManager*	pGM = crea::GameManager::getSingleton();

		// Components
		Json::Value components = root["components"];
		for (unsigned int iComponent = 0; iComponent < components.size(); ++iComponent)
		{
			Json::Value component = components[iComponent];

			string szType = component["type"].asString();
			if (szType == "SpriteRenderer")
			{
				string szName = component["name"].asString();
				SpriteRenderer* pSpriteRenderer = pGM->getSpriteRenderer(szName);

				string szSprite = component["sprite"].asString();
				Sprite* pSprite = pGM->getSprite(szSprite);

				string szTexture = component["image"].asString();
				szTexture = szTexture.substr(szTexture.find_last_of("/"));
				Texture* pTexture = pGM->getTexture(szTexture);
				pSprite->setTexture(pTexture);

				pSpriteRenderer->setSprite(pSprite);
				addComponent(pSpriteRenderer);
			}
			else if (szType == "Agent")
			{
				string szName = component["name"].asString();
				Agent* pAgent = pGM->getAgent(szName);

				addComponent(pAgent);
			}

		}
		return true;
	}

} // namespace crea
