all: library demos



library:
	$(MAKE) -j 4 -C ./build/lib


demos:
	$(MAKE) -C ./build/Examples/1-Rectangles
	$(MAKE) -C ./build/Examples/2-Console
	$(MAKE) -C ./build/Examples/3-Input
	$(MAKE) -C ./build/Examples/4-Images
	$(MAKE) -C ./build/Examples/5-SoundEffects
	$(MAKE) -C ./build/Examples/6-Particles
	$(MAKE) -C ./build/Examples/7-Strings
	$(MAKE) -C ./build/Examples/8-Model
	$(MAKE) -C ./build/Examples/9-Lighting
	$(MAKE) -C ./build/Examples/10-Shaders
	$(MAKE) -C ./build/Examples/11-Camera

clean:
	$(MAKE) clean -C ./build/lib
	$(MAKE) clean -C ./build/Examples/1-Rectangles
	$(MAKE) clean -C ./build/Examples/2-Console
	$(MAKE) clean -C ./build/Examples/3-Input
	$(MAKE) clean -C ./build/Examples/4-Images
	$(MAKE) clean -C ./build/Examples/5-SoundEffects
	$(MAKE) clean -C ./build/Examples/6-Particles
	$(MAKE) clean -C ./build/Examples/7-Strings
	$(MAKE) clean -C ./build/Examples/8-Model
	$(MAKE) clean -C ./build/Examples/9-Lighting
	$(MAKE) clean -C ./build/Examples/10-Shaders
	$(MAKE) clean -C ./build/Examples/11-Camera

