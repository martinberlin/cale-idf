set(srcs 
  "src/AnimatedGIF.cpp"
  "src/gif.inl"
)
idf_component_register(SRCS ${srcs}      
		REQUIRES "AnimatedGIF"
                    INCLUDE_DIRS "src"
)
