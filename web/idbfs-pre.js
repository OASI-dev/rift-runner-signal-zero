/* Keep the personal profile/scoreboard in browser IndexedDB between visits. */
var Module = typeof Module !== "undefined" ? Module : {};
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
  if (typeof FS === "undefined" || typeof IDBFS === "undefined") return;
  if (!FS.analyzePath("/persist").exists) FS.mkdir("/persist");
  FS.mount(IDBFS, {}, "/persist");
  var dependency = "rift-runner-profile-sync";
  addRunDependency(dependency);
  FS.syncfs(true, function (error) {
    if (error) console.warn("RIFT RUNNER: browser save could not be restored", error);
    removeRunDependency(dependency);
  });
});
