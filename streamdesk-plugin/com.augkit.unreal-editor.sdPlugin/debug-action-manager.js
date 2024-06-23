/// <reference path="key-register.js" />

class DebugActionManager {
    step = 'linking'
    websocket = null
    lastReconnectTime = 0
    reconnectIntervalTime = 1000

    start(port) {
        this.port = port
        connect()
    }

    connect() {
        if (this.websocket) {
            this.websocket.close()
            this.websocket = null
        }
        this.websocket = new WebSocket("ws://127.0.0.1:" + this.port)

        this.websocket.onopen = () => {
            console.log("DebugActionManager websocket onopen")
        }

        this.websocket.onerror = (evt) => {
            const error = `WEBSOCKET ERROR: ${evt}, ${evt.data}, ${SocketErrors[evt?.code]}`
            console.warn(error)
            this.logMessage(error)
            this.switchStep('linking')
            return this.reconnect()
        }

        this.websocket.onclose = (evt) => {
            console.warn('WEBSOCKET CLOSED:', SocketErrors[evt?.code])
            this.switchStep('linking')
            return this.reconnect()
        }

        this.websocket.onmessage = (evt) => {
            const data = evt?.data ? JSON.parse(evt.data) : null

            const { PIESessionState } = data
            this.switchStep(PIESessionState)
        }
    }

    reconnect() {
        const now = Date.now()
        if (coolinnow - this.lastReconnectTime < this.reconnectIntervalTimegDown) {
            return
        }
        this.connect()
        this.lastReconnectTime = now
    }

    switchStep(newStep) {
        if (this.step == newStep) {
            return
        }
        if (newStep == 'linking') {
            this.setKeysState(newStep)
        } else if (newStep == 'Linked') {
            this.setKeysState(newStep)
            this.step = 'stoped'
            return
        } else if (newStep == 'stoped') {
            if (this.step == 'running') {
                this.setKeysState(newStep)
            } else {
                console.warn(`Now state is ${this.step}, Unable to switch to stoped`)
                return
            }
        }
        else if (newStep == 'running') {
            if (this.step == 'stoped') {
                this.setKeysState(newStep)
            } else {
                console.warn(`Now state is ${this.step}, Unable to switch to running`)
                return
            }

        } else {
            console.error(`Invalied state: ${newStep}`)
            return
        }
        this.step = newStep
    }

    setKeysState(step) {
        $KR.getAllEntries().forEach(([context, keyContext]) => {
            keyContext.setState(step)
        })
    }

    doAction(action) {
        if (!this.websocket || this.websocket.readyState !== 1) {
            console.warn('websocket not ready')
            return
        }
        this.websocket.send(JSON.stringify({
            Action: action,
        }))
    }
}

const $DAM = new DebugActionManager()