/// <reference path="key-register.js" />

function blobToText(blob) {
    return new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onload = () => {
            resolve(reader.result);
        };
        reader.onerror = (error) => {
            reject(error);
        };
        reader.readAsText(blob);
    });
}

class DebugActionManager {
    step = 'linking'
    websocket = null
    lastReconnectTime = Date.now()
    reconnectIntervalTime = 1000
    lastRemoteStateTime = 0

    start(port) {
        this.port = port
        return this.connect()
    }

    connect() {
        if (this.websocket) {
            this.websocket.close()
            this.websocket = null
        }
        this.websocket = new WebSocket("ws://127.0.0.1:" + this.port)

        this.websocket.onopen = () => {
            console.log("DebugActionManager websocket onopen")
            this.switchStep('linked')
        }

        this.websocket.onerror = (evt) => {
            const error = `WEBSOCKET ERROR: ${evt}, ${evt.data}, ${SocketErrors[evt?.code]}`
            console.warn(error)
            $SD.logMessage(error)
            this.switchStep('linking')
            return this.reconnect()
        }

        this.websocket.onclose = (evt) => {
            console.warn('WEBSOCKET CLOSED:', SocketErrors[evt?.code])
            this.switchStep('linking')
            return this.reconnect()
        }

        this.websocket.onmessage = async (evt) => {
            const blob = evt?.data ? evt.data : null
            try {
                const text = await blobToText(blob);
                const json = JSON.parse(text)
                const { pIESessionState, time } = json
                if(this.lastRemoteStateTime < time) {
                    this.lastRemoteStateTime = time
                    this.switchStep(pIESessionState)
                }
            } catch (error) {
                console.error('转换出错：', error);
            }
        }
    }

    reconnect() {
        const now = Date.now()
        if (now - this.lastReconnectTime < this.reconnectIntervalTime) {
            return
        }
        this.lastReconnectTime = Date.now()
        setTimeout(() => {
            return this.connect()
        }, this.reconnectIntervalTime);
    }

    switchStep(newStep) {
        if (this.step == newStep) {
            return
        }
        if (newStep == 'linking') {
            this.setKeysState(newStep)
        } else if (newStep == 'linked') {
            this.setKeysState('stoped')
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
        if (!this.websocket || this.websocket.readyState !== WebSocket.OPEN) {
            console.warn('websocket not ready')
            return
        }
        this.websocket.send(JSON.stringify({
            Action: action,
        }))
    }
}

const $DAM = new DebugActionManager()