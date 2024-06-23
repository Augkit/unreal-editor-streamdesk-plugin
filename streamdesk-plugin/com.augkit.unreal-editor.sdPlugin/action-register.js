class ActionRegister {
    contextMap = {}
    register(context, action) {
        this.contextMap[context] = action
    }
    unregister(context) {
        delete this.contextMap[context]
    }
    getAllContext() {
        return Object.keys(this.contextMap)
    }
    getAllEntries(){
        return Object.entries(this.contextMap)
    }
}

const $AR = new ActionRegister()
