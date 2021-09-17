// Defines how many Times you can use Engine::TextureLoad, before you have to make room to load new textures
// Textures created via TextureClone are not part of this cap
#define ENGINE_TEXTURE_LIMIT 512

// Defines how many DrawCalls/Objects can be created, before the Engine has allocate more memory (sloooowwwwww) 
// DrawCalls are created by all SubmitForRender-Functions, as well as all "Draw..." functions 
#define ENGINE_DRAW_CALL_LIMIT 1024
