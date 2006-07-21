
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_COMPOUND_H
#define EQS_COMPOUND_H

#include "channel.h"

#include <eq/client/projection.h>
#include <eq/client/range.h>
#include <eq/client/viewMatrix.h>
#include <eq/client/viewport.h>
#include <eq/client/wall.h>

#include <iostream>
#include <vector>

namespace eqs
{
    class Frame;
    class SwapBarrier;

    enum TraverseResult
    {
        TRAVERSE_CONTINUE,
        TRAVERSE_TERMINATE
    };

    /**
     * The compound.
     */
    class Compound
    {
    public:
        /** 
         * Constructs a new Compound.
         */
        Compound();

        /**
         * Constructs a new, deep copy of the passed compound
         */
        Compound( const Compound& from );

        /**
         * Compound tasks define the actions executed by a compound on its
         * channel during compound update, in the order they are defined.
         *
         * The enums are spaced apart to leave room for future additions without
         * breaking binary backward compatibility.
         */
        enum Task
        {
            TASK_NONE     = 0,
            TASK_CLEAR    = 0x1,     //!< Clear the framebuffer
            TASK_CULL     = 0x10,    //!< Cull data
            TASK_DRAW     = 0x100,   //!< Draw data to the framebuffer
            TASK_ASSEMBLE = 0x1000,  //!< Combine input frames
            TASK_READBACK = 0x10000, //!< Read results to output frames
            TASK_ALL      = 0xfffffff
        };

        /**
         * @name Data Access
         */
        //*{
        /** 
         * Adds a new child to this compound.
         * 
         * @param child the child.
         */
        void addChild( Compound* child );

        /** 
         * Removes a child from this compound.
         * 
         * @param child the child
         * @return <code>true</code> if the child was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeChild( Compound* child );

        /** 
         * Returns the number of children on this compound.
         * 
         * @return the number of children on this compound. 
         */
        uint32_t nChildren() const { return _children.size(); }

        /** @return if the compound is a leaf compound. */
        bool isLeaf() const { return _children.empty(); }

        /** 
         * Gets a child.
         * 
         * @param index the child's index. 
         * @return the child.
         */
        Compound* getChild( const uint32_t index ) const
            { return _children[index]; }

        /** @return the parent compound. */
        Compound* getParent() const
            { return _parent; }

        /** @return the root of the compound tree. */
        Compound* getRoot()
            { return _parent ? _parent->getRoot() : this; }

        void setName( const std::string& name ) { _name = name; }
        const std::string& getName() const      { return _name; }

        /** 
         * Set the channel of this compound.
         *
         * The compound uses the channel for all rendering operations executed
         * by this compound.
         * 
         * @param channel the channel.
         */
        void setChannel( Channel* channel ){ _data.channel = channel; }

        /** 
         * Returns the channel of this compound.
         * 
         * @return the channel of this compound.
         */
        Channel* getChannel() const { return _data.channel; }

        Window* getWindow() const 
            { return _data.channel ? _data.channel->getWindow() : NULL; }

        /** 
         * Set the tasks to be executed by the compound.
         * 
         * Previously set tasks are overwritten.
         *
         * @param tasks the compound tasks.
         */
        void setTasks( const uint32_t tasks ) { _tasks = tasks; }

        /** 
         * Add tasks to be executed by the compound.
         *
         * Previously set tasks are preserved.
         * 
         * @param tasks the compound tasks.
         */
        void enableTasks( const uint32_t tasks ) { _tasks |= tasks; }

        /** @return the tasks executed by this compound. */
        uint32_t getTasks() const { return _tasks; }

        /** @return true if the task is set, false if not. */
        bool testTask( const Task task ) const { return (_tasks & task); }

        void setViewport( const eq::Viewport& vp ) { _data.vp = vp; }
        const eq::Viewport& getViewport() const { return _data.vp; }

        void setRange( const eq::Range& range ) { _data.range = range; }
        const eq::Range& getRange() const { return _data.range; }
        //*}

        /** @name IO object access. */
        //*{
        /** 
         * Set a swap barrier.
         *
         * Windows of compounds with the same swap barrier name will enter a
         * barrier before executing eq::Window::swap. Setting an empty string
         * disables the swap barrier.
         * 
         * @param barrier the swap barrier.
         */
        void setSwapBarrier( SwapBarrier* barrier );
        
        /** @return the current swap barrier. */
        SwapBarrier* getSwapBarrier() const { return _swapBarrier; }

        /** 
         * Add a new input frame for this compound.
         *
         * The task parameter is currently ignored, since right now input frames
         * are only used for the assemble task.
         * 
         * @param task The task using the input frame.
         * @param frame the input frame.
         */
        void addInputFrame( Frame* frame, const Task task = TASK_ASSEMBLE )
            { _inputFrames.push_back( frame ); }

        /** 
         * Add a new output frame for this compound.
         *
         * The task parameter is currently ignored, since right now output
         * frames are only used for the readback task.
         * 
         * @param task The task using the output frame.
         * @param frame the output frame.
         */
        void addOutputFrame( Frame* frame, const Task task = TASK_READBACK )
            { _outputFrames.push_back( frame ); }
        //*}

        /**
         * @name View Operations
         */
        //*{
        /** 
         * Set the compound's view using a wall description.
         * 
         * @param wall the wall description.
         */
        void setWall( const eq::Wall& wall );
        
        /** @return the last specified wall description. */
        const eq::Wall& getWall() const { return _view.wall; }

        /** 
         * Set the compound's view using a projection description
         * 
         * @param projection the projection description.
         */
        void setProjection( const eq::Projection& projection );

        /** @return the last specified projection description. */
        const eq::Projection& getProjection() const { return _view.projection; }

        /** 
         * Set the compound's view as a four-by-four matrix.
         * 
         * @param view the view description.
         */
        void setViewMatrix( const eq::ViewMatrix& view  );

        /** @return the last specified projection matrix. */
        const eq::ViewMatrix& getViewMatrix() const { return _view.matrix; }
        //*}

        /** @name Compound Operations. */
        //*{
        typedef TraverseResult (*TraverseCB)(Compound* compound,void* userData);

        /** 
         * Traverses a compound tree in a top-down, left-to-right order.
         * 
         * @param compound the top level compound of the tree to traverse.
         * @param preCB the callback to execute for non-leaf compounds when
         *              traversing down, can be <code>NULL</code>.
         * @param leafCB the callback to execute for leaf compounds, can be
         *               <code>NULL</code>
         * @param postCB the callback to execute for non-leaf compounds when
         *              traversing up, can be <code>NULL</code>.
         * @param userData an opaque pointer passed through to the callbacks.
         *
         * @return the result of the traversal, <code>TRAVERSE_TERMINATE</code>
         *         if the traversal was terminated by one of the callbacks, 
         *         otherwise <code>TRAVERSE_CONTINUE</code> .
         */
        static TraverseResult traverse( Compound* compound, TraverseCB preCB,
                                        TraverseCB leafCB, TraverseCB postCB,
                                        void* userData );

        /** 
         * Initialises this compound.
         */
        void init();

        /** 
         * Exits this compound.
         */
        void exit();

        /** 
         * Updates this compound.
         * 
         * The compound's parameters for the next frame are computed.
         */
        void update();

        /** 
         * Update a channel by generating all rendering tasks for this frame.
         * 
         * @param channel the channel to update.
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         */
        void updateChannel( Channel* channel, const uint32_t frameID );
        //*}

    private:
        std::string _name;

        Compound               *_parent;
        std::vector<Compound*>  _children;
        
        Compound* _getNext() const;

        uint32_t _tasks;

        struct View
        {
            enum Type
            {
                NONE,
                WALL,
                PROJECTION,
                VIEWMATRIX
            };

            View() : latest( NONE ) {}

            Type           latest;
            eq::Wall       wall;
            eq::Projection projection;
            eq::ViewMatrix matrix;
        } 
        _view;

        struct InheritData
        {
            InheritData();

            Channel*       channel;
            eq::Viewport   vp;
            eq::Range      range;
            eq::ViewMatrix view;
        };

        InheritData _data;
        InheritData _inherit;

        SwapBarrier* _swapBarrier;

        std::vector<Frame*> _inputFrames;
        std::vector<Frame*> _outputFrames;
        
        static TraverseResult _initCB( Compound* compound, void* );

        static TraverseResult _updateDrawCB(Compound* compound, void* );
        static TraverseResult _updatePostDrawCB( Compound* compound, void* );

        struct UpdateData
        {
            eqBase::StringHash<eqNet::Barrier*> swapBarriers;
        };

        static TraverseResult _updateCB( Compound* compound, void* userData );
        void _updateInheritData();
        void _updateSwapBarrier( UpdateData* data );
        void _computeFrustum( eq::Frustum& frustum, float headTransform[16] );

        friend std::ostream& operator << ( std::ostream& os,
                                           const Compound* compound );
    };

    std::ostream& operator << ( std::ostream& os,const Compound* compound );
};
#endif // EQS_COMPOUND_H
