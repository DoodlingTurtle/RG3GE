#pragma once


namespace RG3GE {

  class Engine;

	class Scene {

	public:
		Scene( bool persistent = false );
		virtual ~Scene();

		virtual	bool onUpdate(float deltaTime);
		virtual void onDraw(Engine* engine);

		virtual Scene* nextScene() = 0;

		virtual void onStart(Engine* engine);
		virtual void onEnd();

	protected:
		bool persistent;

	private:
		friend class Engine;

	};

}
