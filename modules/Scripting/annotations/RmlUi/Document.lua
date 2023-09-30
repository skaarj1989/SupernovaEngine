---@meta

---@enum ui.ModalFlag
ui.ModalFlag = {
  None = 0,  ---Remove modal state.
  Modal = 1, ---Set modal state, other documents cannot receive focus.
  Keep = 2,  ---Modal state unchanged.
}

---@enum ui.FocusFlag
ui.FocusFlag = {
  None = 0,     --- No focus.
  Document = 1, ---Focus the document.
  Keep = 2,     ---Focus the element in the document which last had focus.
  Auto = 3,     ---Focus the first tab element with the 'autofocus' attribute or else the document.
}

---@class ui.ElementDocument : ui.Element
ui.ElementDocument = {}

---Returns the document's context.
---@return ui.Context
function ui.ElementDocument:getContext() end

---Sets the document's title.
---@param title string
function ui.ElementDocument:setTitle(title) end

---Returns the title of this document.
---@return string
function ui.ElementDocument:getTitle() end

---Returns the source address of this document.
---@return string
function ui.ElementDocument:getSourceURL() end

---Returns the document's compiled style sheet.<br/>
---@return ui.StyleSheet
---NOTE: The style sheet may be regenerated when media query parameters change, invalidating the pointer.
function ui.ElementDocument:getStyleSheet() end

---Reload the document's style sheet from source files.
---Styles will be reloaded from \<style> tags and external style sheets, but not inline 'style' attributes.<br/>
---NOTE: The source url originally used to load the document must still be a valid RML document.
function ui.ElementDocument:reloadStyleSheet() end

---Brings the document to the front of the document stack.
function ui.ElementDocument:pullToFront() end

---Sends the document to the back of the document stack.
function ui.ElementDocument:pushToBack() end

---Show the document.
---@param modalFlag? ui.ModalFlag # Flags controlling the modal state of the document, see the 'ModalFlag' description for details.
---@param focusFlag? ui.FocusFlag # Flags controlling the focus, see the 'FocusFlag' description for details.
function ui.ElementDocument:show(modalFlag, focusFlag) end

---Hide the document.
function ui.ElementDocument:hide() end

---Close the document.<br/>
---NOTE: The destruction of the document is deferred until the next call to Context::Update().
function ui.ElementDocument:close() end

---Creates the named element.
---@param name string # The tag name of the element.
---@return ui.Element
function ui.ElementDocument:createElement(name) end

---Create a text element with the given text content.
---@param text string # The text content of the text element.
---@return ui.Element
function ui.ElementDocument:createTextNode(text) end

---Does the document have modal display set.
---@return boolean # True if the document is hogging focus.
function ui.ElementDocument:isModal() end

---Updates the document, including its layout. Users must call this manually before requesting information such as
---size or position of an element if any element in the document was recently changed, unless Context::Update has
---already been called after the change. This has a perfomance penalty, only call when necessary.
function ui.ElementDocument:updateDocument() end
