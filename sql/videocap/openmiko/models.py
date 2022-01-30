from django.db import models


class User(models.Model):
    email = models.CharField(max_length=256)
    password = models.CharField(max_length=256)


class Session(models.Model):
    user = models.ForeignKey("User", on_delete=models.CASCADE)
    session_key = models.CharField(max_length=40, primary_key=True)
    session_data = models.TextField()
    expire_date = models.DateTimeField(db_index=True)


class AeStrategy(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class AntiflickerAttr(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class AntiFogAttr(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class ColorfxMode(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class DrcMode(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class MeshShadingScale(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class RunningMode(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class SceneMode(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class TemperMode(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class TuningMode(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class TuningOpsMode(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class TuningOpsType(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class CoreAwbStatsMode(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class CoreExposureMode(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class CoreExposureUnit(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class CoreWhiteBalanceMode(models.Model):
    name = models.CharField(max_length=64)
    description = models.TextField()
    enum = models.IntegerField()


class CameraProfile(models.Model):
    ae_strategy = models.ForeignKey("AeStrategy", on_delete=models.CASCADE, null=True)
    anti_flicker_attr = models.ForeignKey(
        "AntiflickerAttr", on_delete=models.CASCADE, null=True
    )
    anti_fog_attr = models.ForeignKey(
        "AntiFogAttr", on_delete=models.CASCADE, null=True
    )
    color_fx_mode = models.ForeignKey(
        "ColorfxMode", on_delete=models.CASCADE, null=True
    )
    drc_mode = models.ForeignKey("DrcMode", on_delete=models.CASCADE, null=True)
    mesh_shading_scale = models.ForeignKey(
        "MeshShadingScale", on_delete=models.CASCADE, null=True
    )
    running_mode = models.ForeignKey("RunningMode", on_delete=models.CASCADE, null=True)
    scene_mode = models.ForeignKey("SceneMode", on_delete=models.CASCADE, null=True)
    temper_mode = models.ForeignKey("TemperMode", on_delete=models.CASCADE, null=True)
    tuning_mode = models.ForeignKey("TuningMode", on_delete=models.CASCADE, null=True)
    tuning_ops_mode = models.ForeignKey(
        "TuningOpsMode", on_delete=models.CASCADE, null=True
    )
    core_awb_stats_mode = models.ForeignKey(
        "CoreAwbStatsMode", on_delete=models.CASCADE, null=True
    )
    core_exposure_mode = models.ForeignKey(
        "CoreExposureMode", on_delete=models.CASCADE, null=True
    )
    core_exposure_unit = models.ForeignKey(
        "CoreExposureUnit", on_delete=models.CASCADE, null=True
    )
    core_white_balance_mode = models.ForeignKey(
        "CoreWhiteBalanceMode", on_delete=models.CASCADE, null=True
    )
