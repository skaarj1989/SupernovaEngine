---@meta

---@class ui.Context
ui.Context = {}

---Returns the name of the context.
---@return string # The context's name.
function ui.Context:getName() end

---Changes the dimensions of the context.
---@param dimensions ivec2 # The new dimensions of the context.
function ui.Context:setDimensions(dimensions) end

---Returns the dimensions of the context.
---@return ivec2 # The current dimensions of the context.
function ui.Context:getDimensions() end

---Creates a new, empty document and places it into this context.
---@param instancerName string # The name of the instancer used to create the document.
---@return ui.ElementDocument # The new document, or nullptr if no document could be created.
function ui.Context:createDocument(instancerName) end

---Load a document into the context.
---@param documentPath string # The path to the document to load. The path is passed directly to the file interface which is used to load the file. The default file interface accepts both absolute paths and paths relative to the working directory.
---@return ui.ElementDocument # The loaded document, or nullptr if no document was loaded.
function ui.Context:loadDocument(documentPath) end

---Unload the given document.
---@param document ui.ElementDocument # The document to unload.
---The destruction of the document is deferred until the next call to Context::Update().
function ui.Context:unloadDocument(document) end

---Unloads all loaded documents.<br/>
---The destruction of the documents is deferred until the next call to Context::Update().
function ui.Context:unloadAllDocuments() end

---Activate or deactivate a media theme. Themes can be used in RCSS media queries.
---@param themeName string # The name of the theme to (de)activate.
---@param activate boolean # True to activate the given theme, false to deactivate.
function ui.Context:activateTheme(themeName, activate) end

---Check if a given media theme has been activated.
---@param themeName string # The name of the theme.
---@return boolean # True if the theme is activated.
function ui.Context:isThemeActive(themeName) end

---Returns the first document in the context with the given id.
---@param id string # The id of the desired document.
---@return ui.ElementDocument # The document (if it was found), or nullptr if no document exists with the ID.
---@overload fun(index: number): ui.ElementDocument
function ui.Context:getDocument(id) end

---Returns the number of documents in the context.
---@return integer
function ui.Context:getNumDocuments() end

---Returns the hover element.
---@return ui.Element # The element the mouse cursor is hovering over.
function ui.Context:getHoverElement() end

---Returns the focus element.
---@return ui.Element # The element with input focus.
function ui.Context:getFocusElement() end

---Returns the root element that holds all the documents
---@return ui.Element # The root element.
function ui.Context:getRootElement() end

---Returns the youngest descendent of the given element which is under the given point in screen coordinates.
---@param point vec2 # The point to test.
---@param ignoreElement? ui.Element # If set, this element and its descendents will be ignored.
---@param element? ui.Element # Used internally.
---@return ui.Element # The element under the point, or nullptr if nothing is.
function ui.Context:getElementAtPoint(point, ignoreElement, element) end

---Brings the document to the front of the document stack.
---@param document ui.ElementDocument # The document to pull to the front of the stack.
function ui.Context:pullDocumentToFront(document) end

---Sends the document to the back of the document stack.
---@param document ui.ElementDocument # The document to push to the bottom of the stack.
function ui.Context:pushDocumentToBack(document) end

---Remove the document from the focus history and focus the previous document.
---@param document ui.ElementDocument # The document to unfocus.
function ui.Context:unfocusDocument(document) end

---Adds an event listener to the context's root element.
---@param event string # The name of the event to attach to.
---@param listener ui.EventListener # Listener object to be attached.
---@param inCapturePhase? boolean # True if the listener is to be attached to the capture phase, false for the bubble phase.
function ui.Context:addEventListener(event, listener, inCapturePhase) end

---Removes an event listener from the context's root element.
---@param event string # The name of the event to detach from.
---@param listener ui.EventListener # Listener object to be detached.
---@param inCapturePhase? boolean # True to detach from the capture phase, false from the bubble phase.
function ui.Context:removeEventListener(event, listener, inCapturePhase) end

---Returns a hint on whether the mouse is currently interacting with any elements in this context, based on previously submitted 'ProcessMouse...()' commands.
---@return boolean # True if the mouse hovers over or has activated an element in this context, otherwise false.
---Interaction is determined irrespective of background and opacity. See the RCSS property 'pointer-events' to disable interaction for specific elements.
function ui.Context:isMouseInteracting() end

---@param name string
---@param table table
---@return ui.ScriptDataModel
function ui.Context:bindDataModel(name, table) end
