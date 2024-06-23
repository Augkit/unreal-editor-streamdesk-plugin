/// <reference path="action-register.js" />

class DebugActionManager {
    state = 'linking'
    websocket = null

    connect(port) {

        if (this.websocket) {
            this.websocket.close()
            this.websocket = null
        }
        this.websocket = new WebSocket("ws://127.0.0.1:" + port)

        this.websocket.onopen = () => {
            console.log("DebugActionManager websocket onopen")
        }

        this.websocket.onerror = (evt) => {
            const error = `WEBSOCKET ERROR: ${evt}, ${evt.data}, ${SocketErrors[evt?.code]}`
            console.warn(error)
            this.logMessage(error)
            switchState('linking')
        }

        this.websocket.onclose = (evt) => {
            console.warn('WEBSOCKET CLOSED:', SocketErrors[evt?.code])
            switchState('linking')
        }

        this.websocket.onmessage = (evt) => {
            const data = evt?.data ? JSON.parse(evt.data) : null

            const { PIESessionState } = data
            switchState(PIESessionState)
        }
    }

    isEnableWhenRunning(action) {
        return action.enableState == 'running'
    }
    isEnableWhenStopped(action) {
        return action.enableState == 'stoped'
    }

    switchState(newState) {
        if (this.state == newState) {
            return
        }
        if (newState == 'linking') {
            $AR.getAllEntries().forEach(([context, action]) => {
                $SD.setState(context, 1)
            })
        } else if (newState == 'Linked') {
            this.doStop()
            this.state = 'stoped'
            return
        } else if (newState == 'stoped') {
            if (this.state == 'running') {
                this.doStop()
            } else {
                console.warn(`Now state is ${this.state}, Unable to switch to stoped`)
                return
            }
        }
        else if (newState == 'running') {
            if (this.state == 'stoped') {
                this.doRun()
            } else {
                console.warn(`Now state is ${this.state}, Unable to switch to running`)
                return
            }

        } else {
            console.error(`Invalied state: ${newState}`)
            return
        }
        this.state = newState
    }

    doStop() {
        $AR.getAllEntries().forEach(([context, action]) => {
            if (this.isEnableWhenRunning(action)) {
                $SD.setState(context, 1)
            } else if (this.isEnableWhenStopped(action)) {
                $SD.setState(context, 0)
            }
        })
    }
    doRun() {
        $AR.getAllEntries().forEach(([context, action]) => {
            if (this.isEnableWhenRunning(action)) {
                $SD.setState(context, 0)
            } else if (this.isEnableWhenStopped(action)) {
                $SD.setState(context, 1)
            }
        })
    }

    doAction(action) {
        if(!this.websocket || this.websocket.readyState !== 1){
            console.warn('websocket not ready')
            return
        }
        this.websocket.send(JSON.stringify({
            Action: action,
        }))
    }
}

const $DAM = new DebugActionManager()