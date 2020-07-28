#ifndef _RIVE_KEYED_OBJECT_HPP_
#define _RIVE_KEYED_OBJECT_HPP_
#include "generated/animation/keyed_object_base.hpp"
#include <vector>
namespace rive
{
	class Artboard;
	class KeyedProperty;
	class KeyedObject : public KeyedObjectBase
	{
	private:
		std::vector<KeyedProperty*> m_KeyedProperties;

	public:
		~KeyedObject();
		void addKeyedProperty(KeyedProperty* property);

		void onAddedDirty(CoreContext* context) override;
		void onAddedClean(CoreContext* context) override;
		void apply(Artboard* coreContext, float time, float mix);
	};
} // namespace rive

#endif