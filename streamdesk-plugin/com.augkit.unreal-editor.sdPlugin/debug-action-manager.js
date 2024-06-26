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
    defaultIP = '127.0.0.1'
    defaultPort = '35346'
    ip = null
    port = null
    websocket = null
    waitingReconnect = false
    reconnectIntervalTime = 1000
    step = 'linking'
    lastRemoteStateTime = 0
    lastRemoteState = null

    start  (ip, port, force = false)  {
        ip = (ip || this.defaultIP)
        port = (port || this.defaultPort)
        if (!force && ip === this.ip && port === this.port) {
            return
        }
        this.ip = ip
        this.port = port
        return this.tryConnect(force)
    }

    connect ()  {
        this.websocket = new WebSocket(`ws://${this.ip}:${this.port}`)

        this.websocket.onopen = () => {
            console.log("DebugActionManager websocket onopen")
            this.switchStep('linked')
        }

        this.websocket.onerror = (evt) => {
            this.switchStep('linking')
            return this.tryConnect()
        }

        this.websocket.onmessage = async (evt) => {
            const blob = evt?.data ? evt.data : null
            try {
                const text = await blobToText(blob);
                const json = JSON.parse(text)
                const { pIESessionState, time } = json
                if (this.lastRemoteStateTime < time) {
                    this.lastRemoteStateTime = time
                    this.lastRemoteState = json
                    this.switchStep(pIESessionState)
                }
            } catch (error) {
                console.error(error);
            }
        }
    }

    tryConnect (force = false)  {
        if (!force && this.waitingReconnect) {
            return
        }
        if (force) {
            if (this.websocket) {
                this.websocket.close()
                this.websocket = null
            }
            return this.connect()
        } else {
            setTimeout(() => {
                this.waitingReconnect = false
                if (force && !!this.websocket && (this.websocket.readyState === WebSocket.OPEN || this.websocket.readyState === WebSocket.CONNECTING)) {
                    this.websocket.close()
                    this.websocket = null
                }
                return this.connect()
            }, this.reconnectIntervalTime);
        }
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

    getPIESessionType() {
        return this.lastRemoteState?.pIESessionType || ''
    }
}

const $DAM = new DebugActionManager()