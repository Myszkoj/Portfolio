[RULES AND FUNCTIONALITY]
1) An object may have any number of parent types.
	1a) All parent types must be assigned to the relation level. (See@ section 2 and 3)
	1b) Relation levels must be declared before definition of the object class. (See@ section 2 and 3)
	1c) When an object is constructed, only one parent type can be selected from the types at the given relation level.
	1d) ParentList must be sorted by relation level(e.g. parents with level 6 must appear before parents of level 8 and 23).
	1e) UnknownParent tag can be used in place of the parent type if the child does not know it.
		This may be useful if access needs to be one-sided, or one wish the hierarchy to be expandable by building more objects on top of it.

2) Relation level is declared with a macro: DECLARE_COMPLEX_RELATION(_CHILD_, _LEVEL_, _REQUIREMENT_, _TYPE_)
	2a) DECLARE_COMPLEX_RELATION must be used in a complex namespace.
	2b) The numbers used for relation levels are arbitrary and does not need to be consecutive, 
		but they may collide with dpl classes, so take that into account if object is derived from dpl: Association/Sequence/Link/Observer/Reference).
	2c) RelationRequirement(OBLIGATORY, or OPTIONAL) indicate if the parent is required for the child to be "alive".
		Objects are destroyed automatically with their OBLIGATORY parents.
	2d) RelationType(ONE_TO_ONE, or ONE_TO_MANY) controlls internal interface of the parent-child relation.
		ONE_TO_ONE allows for creation of the single child, while ONE_TO_MANY does not restrict the number of children.
		Both relation types have different memory overhead.

3) Parent is added to the relation level with macro: EXPAND_COMPLEX_RELATION(_CHILD_, _LEVEL_, _PARENT_)
	3a) Any parent type can only be assigned to a single relation level per child type.

4) An object may have any number of children.
	4a) The number of children of the given type depends on the defined relation level at which the parent type appears.
	4b) A child type can still be used even if the parent type does not appear in this child's list of parent types, 
		as long as child relation with UnknownParent is defined. In this case RuntimeParent type is injected between parent interface and object type.
	4c) ChildList must be sorted by dependency order (See@ section 4)

5) Dependency list is generated for all object types.
	5a) The list contains recursive combinations of object type and the types of it's children.
	5b) The types in the list are filtered to be unique(duplicates are removed).
	5c) The list is then passed recursively to all object types in the hierarchy and checked if all OBLIGATORY parent types appear before the child types(object type may be a parent of itself).

6) In order to make object instantiable one must define Instance<ObjT> in the complex namespace before ObjT definition.
	6a) Instance<ObjT> must be derived from the InstanceBase<ObjT>.
	6b) Instances can be inherited from the parent object of the first relation level.
	6c) Inheritance assigns the same instanceID for all children down the graph and automatically creates and destroys them.
		
7) Undo/redu operations are supported for objects and instances.
	7a) Any operation performed with the project's command buffer can be undone/redone.
	7b) Command buffer is not restricted in size.
	7c) Direct creation/destruction of the objects/instances causes command buffer to be cleared.
	7d) Save/load/open of the project clears the command buffer.

[TODO]:
- Make new project with: System-Object-Instance-Component (soic)
- Disallow relation <ObjT, ObjT> when parent type is unknown.
- Add system groups. (solves glw::Context::Lock problem)
- Move LOCK functionality from Member/Instance to Project.
- Create object directly. (This should probably preserve command buffer)
- Destroy object directly (and all of its children). (clear command buffer)
- Going back to "old" rendering pipeline design creates problem with the glw::Context::Lock.
- Double check ownership transfer method in dpl::Property class. (Maybe there is a better way.)
