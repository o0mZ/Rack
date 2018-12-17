#include "osdialog.h"
#include "system.hpp"
#include "network.hpp"
#include "app/Scene.hpp"
#include "app/ModuleBrowser.hpp"
#include "app/RackScrollWidget.hpp"
#include <thread>


namespace rack {


Scene::Scene() {
	scrollWidget = new RackScrollWidget;
	{
		zoomWidget = new ZoomWidget;
		{
			rackWidget = new RackWidget;
			zoomWidget->addChild(rackWidget);
		}
		scrollWidget->container->addChild(zoomWidget);
	}
	addChild(scrollWidget);

	toolbar = new Toolbar;
	addChild(toolbar);
	scrollWidget->box.pos.y = toolbar->box.size.y;
}

void Scene::step() {
	// Resize owned descendants
	toolbar->box.size.x = box.size.x;
	scrollWidget->box.size = box.size.minus(scrollWidget->box.pos);

	// Resize to be a bit larger than the ScrollWidget viewport
	rackWidget->box.size = scrollWidget->box.size
		.minus(scrollWidget->container->box.pos)
		.plus(Vec(500, 500))
		.div(zoomWidget->zoom);

	OpaqueWidget::step();

	zoomWidget->box.size = rackWidget->box.size.mult(zoomWidget->zoom);

	// Request latest version from server
	if (!devMode && checkVersion && !checkedVersion) {
		std::thread t(&Scene::runCheckVersion, this);
		t.detach();
		checkedVersion = true;
	}

	// Version popup message
	if (!latestVersion.empty()) {
		std::string versionMessage = string::f("Rack %s is available.\n\nYou have Rack %s.\n\nClose Rack and download new version on the website?", latestVersion.c_str(), APP_VERSION.c_str());
		if (osdialog_message(OSDIALOG_INFO, OSDIALOG_OK_CANCEL, versionMessage.c_str())) {
			std::thread t(system::openBrowser, "https://vcvrack.com/");
			t.detach();
			windowClose();
		}
		latestVersion = "";
	}
}

void Scene::draw(NVGcontext *vg) {
	OpaqueWidget::draw(vg);
}

void Scene::onHoverKey(event::HoverKey &e) {
	OpaqueWidget::onHoverKey(e);

	if (!e.target) {
		switch (e.key) {
			case GLFW_KEY_N: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					rackWidget->reset();
					e.target = this;
				}
			} break;
			case GLFW_KEY_Q: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					windowClose();
					e.target = this;
				}
			} break;
			case GLFW_KEY_O: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					rackWidget->loadDialog();
					e.target = this;
				}
				if (windowIsModPressed() && windowIsShiftPressed()) {
					rackWidget->revert();
					e.target = this;
				}
			} break;
			case GLFW_KEY_S: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					rackWidget->saveDialog();
					e.target = this;
				}
				if (windowIsModPressed() && windowIsShiftPressed()) {
					rackWidget->saveAsDialog();
					e.target = this;
				}
			} break;
			case GLFW_KEY_V: {
				if (windowIsModPressed() && !windowIsShiftPressed()) {
					rackWidget->pastePresetClipboard();
					e.target = this;
				}
			} break;
			case GLFW_KEY_ENTER:
			case GLFW_KEY_KP_ENTER: {
				moduleBrowserCreate();
				e.target = this;
			} break;
			case GLFW_KEY_F11: {
				windowSetFullScreen(!windowGetFullScreen());
			}
		}
	}
}

void Scene::onPathDrop(event::PathDrop &e) {
	if (e.paths.size() >= 1) {
		const std::string &path = e.paths[0];
		if (string::extension(path) == "vcv") {
			rackWidget->load(path);
			e.target = this;
		}
	}

	if (!e.target)
		OpaqueWidget::onPathDrop(e);
}

void Scene::runCheckVersion() {
	json_t *resJ = network::requestJson(network::METHOD_GET, API_HOST + "/version", NULL);

	if (resJ) {
		json_t *versionJ = json_object_get(resJ, "version");
		if (versionJ) {
			std::string version = json_string_value(versionJ);
			if (version != APP_VERSION) {
				latestVersion = version;
			}
		}
		json_decref(resJ);
	}
}


Scene *gScene = NULL;


} // namespace rack