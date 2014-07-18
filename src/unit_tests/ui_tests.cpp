#include <cassert>

#define TEST_BUILD //removes external library ties

#include "../ui_elem.h"
#include "../frame.h"
#include "../button.h"
#include "../scrolling_frame.h"

//run with valgrind most testing

int main(){
	{
		printf("one UIElem\n");
		UIElem* elem = new UIElem(5, 5);
		delete elem;
	}

	{
		printf("Frame as UIElem\n");
		UIElem* frame = new Frame(vec2i(100, 100), vec2i(100, 100));
		delete frame;
	}

	{
		printf("Button as UIElem\n");
		UIElem* button = new Button(vec2i(100, 100), (char*)"button");
		delete button;
	}

	{
		printf("ScrollingFrame as UIElem\n");
		ScrollingFrame* sf = new ScrollingFrame(vec2i(10, 10), 100);
		delete sf;
	}

	{
		printf("adding and removing stuff from frame\n");
		Frame* frame = new Frame(vec2i(10, 10), vec2i(100, 100));
		Button* button = new Button(vec2i(50, 0), (char*)"first");
		printf("frame 0x%x, button 0x%x\n", frame, button);

		frame->addChild(button);
		assert(button->getParent() == frame);
		assert(frame->hasChild(button));

		frame->addChild(new Button(vec2i(0, 0), (char*)"second"));
		frame->addChild(new Frame(vec2i(100, 10), vec2i(10, 10)));

		printf("deleting button\n");
		delete button;
		assert(!frame->hasChild(button));
		printf("deleted button\n");

		delete frame;
	}

	return 0;
}
