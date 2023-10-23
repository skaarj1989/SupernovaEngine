---@meta

---@enum ui.ScrollBehavior
ui.ScrollBehavior = {
  Auto = 0,    ---Scroll using the context's configured setting.
  Smooth = 1,  ---Scroll using a smooth animation.
  Instant = 2, ---Scroll instantly.
}

---@enum ui.ScrollAlignment
ui.ScrollAlignment = {
  Start = 0,   ---Align to the top or left edge of the parent element.
  Center = 1,  ---Align to the center of the parent element.
  End = 2,     ---Align to the bottom or right edge of the parent element.
  Nearest = 3, ---Align with minimal scroll change.
}

---@class ui.Element
---@overload fun(tag: string): ui.Element
ui.Element = {}

---Sets or removes a class on the element.
---@param className string # The name of the class to add or remove from the class list.
---@param activate boolean # True if the class is to be added, false to be removed.
function ui.Element:setClass(className, activate) end

---Checks if a class is set on the element.
---@param className string # The name of the class to check for.
---@return boolean # True if the class is set on the element, false otherwise.
function ui.Element:isClassSet(className) end

---Specifies the entire list of classes for this element. This will replace any others specified.
---@param classNames string # The list of class names to set on the style, separated by spaces.
function ui.Element:setClassNames(classNames) end

---Return the active class list.
---@return string # The space-separated list of classes active on the element.
function ui.Element:getClassNames() end

---Returns the active style sheet for this element. This may be nullptr.
---@return ui.StyleSheet # The element's style sheet.
function ui.Element:getStyleSheet() end

---Fills a string with the full address of this element.
---@param includePseudoClasses? boolean # True if the address is to include the pseudo-classes of the leaf element.
---@param includeParents? boolean
---@return string # The address of the element, including its full parentage.
function ui.Element:getAddress(includePseudoClasses, includeParents) end

---Sets the position of this element, as a two-dimensional offset from another element.
---@param offset vec2 # The offset (in pixels) of our primary box's top-left border corner from our offset parent's top-left border corner.
---@param offsetParent ui.Element # The element this element is being positioned relative to.
---@param offsetFixed? boolean # True if the element is fixed in place (and will not scroll), false if not.
function ui.Element:setOffset(offset, offsetParent, offsetFixed) end

---Returns the position of the top-left corne offset parent's top-left border corner.
---@param area? ui.BoxArea # The desired area position.
---@return vec2 # The relative offset.
function ui.Element:getRelativeOffset(area) end

---Returns the position of the top-left corner of one of the areas of this element's primary box, relative to
---the element root.
---@param area? ui.BoxArea # The desired area position.
---@return vec2 # The absolute offset.
function ui.Element:getAbsoluteOffset(area) end

---Sets an alternate area to use as the client area.
---@param clientArea ui.BoxArea # The box area to use as the element's client area.
function ui.Element:setClientArea(clientArea) end

---Returns the area the element uses as its client area.
---@return ui.BoxArea # The box area used as the element's client area.
function ui.Element:getClientArea() end

---Sets the dimensions of the element's scrollable overflow rectangle. This is the tightest fitting box surrounding
---all of this element's logical children, and the element's padding box.
---@param scrollableOverflowRectangle ivec2 # The dimensions of the box's scrollable content.
function ui.Element:setScrollableOverflowRectangle(scrollableOverflowRectangle) end

---Sets the box describing the size of the element, and removes all others.
---@param box ui.Box # The new dimensions box for the element.
function ui.Element:setBox(box) end

---Adds a box to the end of the list describing this element's geometry.
---@param box ui.Box # The auxiliary box for the element.
---@param offset vec2 # The offset of the box relative to the top left border corner of the element.
function ui.Element:addBox(box, offset) end

---Returns one of the boxes describing the size of the element.
---@param index integer # The index of the desired box, with 0 being the main box. If outside of bounds, the main box will be returned.
---@return ui.Box # The requested box.
---@return vec2 offset # The offset of the box relative to the element's border box.
---@overload fun(): ui.Box
function ui.Element:getBox(index) end

---Returns the number of boxes making up this element's geometry.
---@return integer # the number of boxes making up this element's geometry.
function ui.Element:getNumBoxes() end

---Returns the baseline of the element, in pixels offset from the bottom of the element's content area.
---@return number # The element's baseline. A negative baseline will be further 'up' the element, a positive on further 'down'. The default element will return 0.
function ui.Element:getBaseline() end

---Gets the intrinsic dimensions of this element, if it is of a type that has an inherent size. This size will
---only be overriden by a styled width or height.
---@return boolean # True if the element has intrinsic dimensions, false otherwise. The default element will return false.
---@return vec2 dimensions # The dimensions to size, if appropriate.
---@return number ratio # The intrinsic ratio (width/height), if appropriate.
function ui.Element:getIntrinsicDimensions() end

---Returns true if the element is replaced, thereby handling its own rendering.
---@return boolean # True if the element is a replaced element.
function ui.Element:isReplaced() end

---Checks if a given point in screen coordinates lies within the bordered area of this element.
---@param point vec2 # The point to test.
---@return boolean # True if the element is within this element, false otherwise.
function ui.Element:isPointWithinElement(point) end

---Returns the visibility of the element.
---@param includeAncestors? boolean # Check parent elements for visibility
---@return boolean # True if the element is visible, false otherwise.
function ui.Element:isVisible(includeAncestors) end

---Returns the z-index of the element.
---@return number # The element's z-index.
function ui.Element:getZIndex() end

---Returns the element's font face handle.
---@return ui.FontFaceHandle # The element's font face handle.
function ui.Element:getFontFaceHandle() end

---Sets a local property override on the element.
---@param name string # The name of the new property.
---@param value string|ui.Property # The new property to set.
---@return boolean # True if the property parsed successfully, false otherwise.
function ui.Element:setProperty(name, value) end

---Removes a local property override on the element; its value will revert to that defined in the style sheet.
---@param name string|ui.PropertyId # The name of the local property definition to remove.
function ui.Element:removeProperty(name) end

---Returns one of this element's properties. If the property is not defined for this element and not inherited
---from an ancestor, the default value will be returned.
---@param name? string|ui.PropertyId # The name of the property to fetch the value for.
---@return ui.Property # The value of this property for this element, or nullptr if no property exists with the given name.
function ui.Element:getProperty(name) end

---Returns one of this element's properties. If this element is not defined this property, nullptr will be returned.
---@param name string|ui.PropertyId # The name of the property to fetch the value for.
---@return ui.Property # The value of this property for this element, or nullptr if this property has not been explicitly defined for this element.
function ui.Element:getLocalProperty(name) end

---Resolves a length to its canonical unit ('px').
---@param value ui.NumericValue # The numeric value.
---@return number # The resolved value in their canonical unit, or zero if it could not be resolved.
---Font-relative and context-relative units will be resolved against this element's computed values and its context.
function ui.Element:resolveLength(value) end

---Resolves a numeric value with units of number, percentage, length, or angle to their canonical unit (unit-less, 'px', or 'rad').
---Numbers and percentages are scaled by the base value and returned.
---@param value ui.NumericValue # The value to be resolved.
---@param baseValue number # The value that is scaled by the number or percentage value, if applicable.
---@return number # The resolved value in their canonical unit, or zero if it could not be resolved.
function ui.Element:resolveNumericValue(value, baseValue) end

---Returns the size of the containing block. Often percentages are scaled relative to this.
---@return vec2
function ui.Element:getContainingBlock() end

---Returns 'position' property value from element's computed values.
---@return ui.Style.Position
function ui.Element:getPosition() end

---Returns 'float' property value from element's computed values.
---@return ui.Style.Float
function ui.Element:getFloat() end

---Returns 'display' property value from element's computed values.
---@return ui.Style.Display
function ui.Element:getDisplay() end

---Returns 'line-height' property value from element's computed values.
---@return number
function ui.Element:getLineHeight() end

---Project a 2D point in pixel coordinates onto the element's plane.
---@param point vec2 # The point to project in, and the resulting projected point out.
---@return boolean # True on success, false if transformation matrix is singular.
function ui.Element:project(point) end

---Start an animation of the given property on this element.<br/>
---If an animation of the same property name exists, it will be replaced.<br/>
---If startValue is null, the current property value on this element is used.
---@param propertyName string
---@param targetValue ui.Property
---@param duration number
---@param tween? ui.Tween
---@param numIterations? integer
---@param alternateDirection? boolean
---@param delay? number
---@param startValue? ui.Property
---@return boolean # True if a new animation was added.
function ui.Element:animate(propertyName, targetValue, duration, tween, numIterations, alternateDirection, delay, startValue) end

---Add a key to an animation, extending its duration.<br/>
---If no animation exists for the given property name, the call will be ignored.
---@param propertyName string
---@param targetValue ui.Property
---@param duration number
---@param tween? ui.Tween
---@return boolean # True if a new animation key was added.
function ui.Element:addAnimationKey(propertyName, targetValue, duration, tween) end

---Sets or removes a pseudo-class on the element.
---@param pseudoClass string # The pseudo class to activate or deactivate.
---@param activate boolean # True if the pseudo-class is to be activated, false to be deactivated.
function ui.Element:setPseudoClass(pseudoClass, activate) end

---Checks if a specific pseudo-class has been set on the element.
---@param pseudoClass string # The name of the pseudo-class to check for.
---@return boolean # True if the pseudo-class is set on the element, false if not.
function ui.Element:isPseudoClassSet(pseudoClass) end

---Checks if a complete set of pseudo-classes are set on the element.
---@param pseudoClasses string[] # The list of pseudo-classes to check for.
---@return boolean # True if all of the pseudo-classes are set, false if not.
function ui.Element:arePseudoClassesSet(pseudoClasses) end

---Gets a list of the current active pseudo-classes.
---@return string[] # The list of active pseudo-classes.
function ui.Element:getActivePseudoClasses() end

---Gets the specified attribute.
---@param name string # Name of the attribute to retrieve.
---@return ui.Variant
function ui.Element:getAttribute(name) end

---Checks if the element has a certain attribute.
---@param name string # The name of the attribute to check for.
---@return boolean # True if the element has the given attribute, false if not.
function ui.Element:hasAttribute(name) end

---Removes the attribute from the element.
---@param name string # Name of the attribute.
function ui.Element:removeAttribute(name) end

---Returns the number of attributes on the element.
---@return integer # The number of attributes on the element.
function ui.Element:getNumAttributes() end

---Gets the outer-most focus element down the tree from this node.
---@return ui.Element # Outer-most focus element.
function ui.Element:getFocusLeafNode() end

---Returns the element's context.
---@return ui.Context # The context this element's document exists within.
function ui.Element:getContext() end

---Gets the name of the element.
---@return string # The name of the element.
function ui.Element:getTagName() end

---Gets the id of the element.
---@return string # The element's id.
function ui.Element:getId() end

---Sets the id of the element.
---@param id string # The new id of the element.
function ui.Element:setId(id) end

---Gets the horizontal offset from the context's left edge to element's left border edge.
---@return number # The horizontal offset of the element within its context, in pixels.
function ui.Element:getAbsoluteLeft() end

---Gets the vertical offset from the context's top edge to element's top border edge.
---@return number # The vertical offset of the element within its context, in pixels.
function ui.Element:getAbsoluteTop() end

---Gets the horizontal offset from the element's left border edge to the left edge of its client area. This is
---usually the edge of the padding, but may be the content area for some replaced elements.
---@return number # The horizontal offset of the element's client area, in pixels.
function ui.Element:getClientLeft() end

---Gets the vertical offset from the element's top border edge to the top edge of its client area. This is
---usually the edge of the padding, but may be the content area for some replaced elements.
---@return number # he vertical offset of the element's client area, in pixels.
function ui.Element:getClientTop() end

---Gets the width of the element's client area. This is usually the padded area less the vertical scrollbar
---width, but may be the content area for some replaced elements.
---@return number # The width of the element's client area, usually including padding but not the vertical scrollbar width, border or margin.
function ui.Element:getClientWidth() end

---Gets the height of the element's client area. This is usually the padded area less the horizontal scrollbar
---height, but may be the content area for some replaced elements.
---@return number # The inner height of the element, usually including padding but not the horizontal scrollbar height, border or margin.
function ui.Element:getClientHeight() end

---Returns the element from which all offset calculations are currently computed.
---@return ui.Element # This element's offset parent.
function ui.Element:getOffsetParent() end

---Gets the distance from this element's left border to its offset parent's left border.
---@return number # The horizontal distance (in pixels) from this element's offset parent to itself.
function ui.Element:getOffsetLeft() end

---Gets the distance from this element's top border to its offset parent's top border.
---@return number # The vertical distance (in pixels) from this element's offset parent to itself.
function ui.Element:getOffsetTop() end

---Gets the width of the element, including the client area, padding, borders and scrollbars, but not margins.
---@return number # The width of the rendered element, in pixels.
function ui.Element:getOffsetWidth() end

---Gets the height of the element, including the client area, padding, borders and scrollbars, but not margins.
---@return number # The height of the rendered element, in pixels.
function ui.Element:getOffsetHeight() end

---Gets the left scroll offset of the element.
---@return number # The element's left scroll offset.
function ui.Element:getScrollLeft() end

---Sets the left scroll offset of the element.
---@param scrollLeft number # The element's new left scroll offset.
function ui.Element:setScrollLeft(scrollLeft) end

---Gets the top scroll offset of the element.
---@return number # The element's top scroll offset.
function ui.Element:getScrollTop() end

---Sets the top scroll offset of the element.
---@param scrollTop number # The element's new top scroll offset.
function ui.Element:setScrollTop(scrollTop) end

---Gets the width of the scrollable content of the element; it includes the element padding but not its margin.
---@return number # The width (in pixels) of the of the scrollable content of the element.
function ui.Element:getScrollWidth() end

---Gets the height of the scrollable content of the element; it includes the element padding but not its margin.
---@return number # The height (in pixels) of the of the scrollable content of the element.
function ui.Element:getScrollHeight() end

---Gets the document this element belongs to.
---@return number # This element's document.
function ui.Element:getOwnerDocument() end

---Gets this element's parent node.
---@return ui.Element # This element's parent.
function ui.Element:getParentNode() end

---Recursively search for the first ancestor of this node matching the given selector.
---@param selectors string # The selector or comma-separated selectors to match against.
---@return ui.Element # The ancestor if found, or nullptr if no ancestor could be matched.
---PERFORMANCE: Prefer GetElementById/TagName/ClassName whenever possible.
function ui.Element:closest(selectors) end

---Gets the element immediately following this one in the tree.
---@return ui.Element # This element's next sibling element, or nullptr if there is no sibling element.
function ui.Element:getNextSibling() end

---Gets the element immediately preceding this one in the tree.
---@return ui.Element # This element's previous sibling element, or nullptr if there is no sibling element.
function ui.Element:getPreviousSibling() end

---Returns the first child of this element.
---@return ui.Element # This element's first child, or nullptr if it contains no children.
function ui.Element:getFirstChild() end

---Gets the last child of this element.
---@return ui.Element # This element's last child, or nullptr if it contains no children.
function ui.Element:getLastChild() end

---Get the child element at the given index.
---@param index integer # Index of child to get.
---@return ui.Element # The child element at the given index.
function ui.Element:getChild(index) end

---Get the current number of children in this element
---@param includeNonDOMElements boolean # True if the caller wants to include the non DOM children. Only set this to true if you know what you're doing!
---@return integer # The number of children.
function ui.Element:getNumChildren(includeNonDOMElements) end

---Gets the markup and content of the element.
---@return string # The content of the element.
function ui.Element:getInnerRML() end

---Sets the markup and content of the element. All existing children will be replaced.
---@param rml string # The new content of the element.
function ui.Element:setInnerRML(rml) end

---Gives focus to the current element.
---@return boolean # True if the change focus request was successful.
function ui.Element:focus() end

---Removes focus from from this element.
function ui.Element:blur() end

---Fakes a mouse click on this element.
function ui.Element:click() end

---Adds an event listener to this element.
---@param event string|ui.EventId # Event to attach to.
---@param listener ui.EventListener # The listener object to be attached.
---@param inCapturePhase? boolean # True to attach in the capture phase, false in bubble phase.
---LIFETIME: The added listener must stay alive until after the dispatched call from EventListener::onDetach(). This occurs
---eg. when the element is destroyed or when RemoveEventListener() is called with the same parameters passed here.
function ui.Element:addEventListener(event, listener, inCapturePhase) end

---Removes an event listener from this element.
---@param event string|ui.EventId # Event to detach from.
---@param listener ui.EventListener # The listener object to be detached.
---@param inCapturePhase? boolean # True to detach from the capture phase, false from the bubble phase.
function ui.Element:removeEventListener(event, listener, inCapturePhase) end

---Scrolls the parent element's contents so that this element is visible.
---@param alignWithTop boolean # If true, the element will align itself to the top of the parent element's window. If false, the element will be aligned to the bottom of the parent element's window.
function ui.Element:scrollIntoView(alignWithTop) end

---Sets the scroll offset of this element to the given coordinates.
---@param position vec2 # The scroll destination coordinates.
---@param behavior? ui.ScrollBehavior # Smooth scrolling behavior.
---NOTE: Smooth scrolling can only be applied to a single element at a time, any active smooth scrolls will be cancelled.
function ui.Element:scrollTo(position, behavior) end

---Returns whether or not this element has any DOM children.
---@return boolean # True if the element has at least one DOM child, false otherwise.
function ui.Element:hasChildNodes() end

---Get a child element by its ID.
---@param id string # Id of the the child element
---@return ui.Element # The child of this element with the given ID, or nullptr if no such child exists.
function ui.Element:getElementById(id) end

---Get all descendant elements with the given tag.
---@param elements ui.Element[] # Resulting elements.
---@param tag string # Tag to search for.
function ui.Element:getElementsByTagName(elements, tag) end

---Get all descendant elements with the given class set on them.
---@param elements ui.Element[] # Resulting elements.
---@param tag string # Tag to search for.
function ui.Element:getElementsByClassName(elements, tag) end

---Returns the first descendent element matching the RCSS selector query.
---@param selectors ui.Element[] # The selector or comma-separated selectors to match against.
---@return ui.Element # The first matching element during a depth-first traversal.
---PERFORMANCE: Prefer GetElementById/TagName/ClassName whenever possible.
function ui.Element:querySelector(selectors) end

---Returns all descendent elements matching the RCSS selector query.
---@param elements ui.Element[] # The list of matching elements.
---@param selectors string # The selector or comma-separated selectors to match against.
---PERFORMANCE: Prefer GetElementById/TagName/ClassName whenever possible.
function ui.Element:querySelectorAll(elements, selectors) end
