/// <reference path="../../libs/js/property-inspector.js" />
/// <reference path="../../libs/js/utils.js" />

$PI.onConnected((jsn) => {
    const form = document.getElementById('global-setting')
    $PI.getGlobalSettings()
    form.addEventListener('submit', ()=>{
        const value = Utils.getFormValue(form)
        $PI.setGlobalSettings(value)
    })
    form.addEventListener('reset', ()=>{
        $PI.setGlobalSettings({})
    })
})

$PI.onDidReceiveGlobalSettings(({ payload }) => {
    Utils.setFormValue(payload?.settings, document.getElementById('global-setting'))
})