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
    pIESessionType

    constructor(...params) {
        super(...params)
        this.pIESessionType = ''
    }
    setEnableState(step) {
        let isStateChanged = false
        if (this.state !== 'enable') {
            isStateChanged = true
        }
        const lastPIESessionType = $DAM.getPIESessionType()
        let isPIESessionTypeChanged = false
        if (lastPIESessionType !== this.pIESessionType) {
            isPIESessionTypeChanged = true;
        }
        if (!isPIESessionTypeChanged && !isStateChanged) {
            return
        }
        $SD.setState(this.context, this.getActionStateOffset(lastPIESessionType) + 1)
        this.state = 'enable'
        this.pIESessionType = lastPIESessionType;
    }
    setDisableState(step) {
        const lastPIESessionType = $DAM.getPIESessionType()
        if (step === 'running' && this.state !== 'resume') {
            $SD.setState(this.context, 3)
            this.state = 'resume'
        }
        else {
            let isStateChanged = false
            if (this.state !== 'disable') {
                isStateChanged = true
            }
            let isPIESessionTypeChanged = false
            if (lastPIESessionType !== this.pIESessionType) {
                isPIESessionTypeChanged = true;
            }
            $SD.setState(this.context, this.getActionStateOffset(lastPIESessionType))
            this.state = 'disable'
        }
        this.pIESessionType = lastPIESessionType;
    }
    getActionStateOffset(pIESessionType) {
        if (pIESessionType === 'EditorFloating') {
            return 4
        } else if (pIESessionType === 'MobilePreview') {
            return 6
        } else if (pIESessionType === 'VR') {
            return 8
        } else if (pIESessionType === 'Simulate') {
            return 2
        } else if (pIESessionType === 'NewProcess') {
            return 10
        } else {
            return 0
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
        return this.contextMap[context]
    }
    getAllContext() {
        return Object.keys(this.contextMap)
    }
    getAllEntries() {
        return Object.entries(this.contextMap)
    }
    getKeyContetxtState(context) {
        return this.getKeyContetxt(context)?.state
    }
    isKeyEnable(context) {
        const state = this.getKeyContetxtState(context)
        return !!state && state === 'disable'
    }
}

const $KR = new KeyRegister()
