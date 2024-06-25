/// <reference path="libs/js/stream-deck.js" />
/// <reference path="debug-action.js" />

class KeyContext {
    state
    context
    action
    constructor(context, action) {
        this.state = ''
        this.context = context
        this.action = action
    }

    setState(step) {
        if (step === 'running') {
            if (this.action.enableStep === 'running') {
                this.setEnableState(step)
            } else {
                this.setDisableState(step)
            }
        } else if (step === 'stoped') {
            if (this.action.enableStep === 'stoped') {
                this.setEnableState(step)
            } else {
                this.setDisableState(step)
            }
        } else {
            this.setDisableState(step)
        }
    }
    setEnableState(step) {
        if (this.state === 'enable') {
            return
        }
        $SD.setState(this.context, 1)
        this.state = 'enable'
    }
    setDisableState(step) {
        if (this.state === 'disable') {
            return
        }
        $SD.setState(this.context, 0)
        this.state = 'disable'
    }
}

class PlayKeyContext extends KeyContext {
    setDisableState(step) {
        if (step === 'running' && this.state !== 'enable2') {
            $SD.setState(this.context, 2)
            this.state = 'resume'
        }
        else {
            super.setDisableState(step)
        }
    }
}

class KeyRegister {
    contextMap = {}
    register(context, action) {
        if (action.specialKey === 'play') {
            this.contextMap[context] = new PlayKeyContext(context, action)
        } else {
            this.contextMap[context] = new KeyContext(context, action)
        }
        this.contextMap[context].setState($DAM.step)
    }
    unregister(context) {
        delete this.contextMap[context]
    }
    getKeyContetxt(context) {
        return this[context]
    }
    getAllContext() {
        return Object.keys(this.contextMap)
    }
    getAllEntries() {
        return Object.entries(this.contextMap)
    }
    isKeyEnable(context) {
        return contextMap[context]?.state !== 'disable'
    }
}

const $KR = new KeyRegister()
