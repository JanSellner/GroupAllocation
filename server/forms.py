from flask_wtf import FlaskForm
from wtforms import SubmitField, IntegerField, TextAreaField, FileField, RadioField, FloatField, HiddenField
from wtforms.validators import DataRequired, Regexp
from wtforms.widgets.html5 import NumberInput
from flask_wtf.file import FileAllowed


class InputDataForm(FlaskForm):
    n_groups = IntegerField('Number of groups/days', validators=[DataRequired()], widget=NumberInput(min=2, max=8), default=3)
    selection_type = RadioField('Selection type', choices=[('text', 'Specify users manually'), ('file', 'Upload a file')], validators=[DataRequired()], default='text')
    users = TextAreaField('Users', description='One name per line.')
    file = FileField('File', validators=[FileAllowed(['csv'])], description='CSV file')
    alpha1 = FloatField('alpha1', validators=[DataRequired()], widget=NumberInput(min=0, max=1, step='any'), default=1/3)
    alpha2 = FloatField('alpha2', validators=[DataRequired()], widget=NumberInput(min=0, max=1, step='any'), default=1/3)
    alpha3 = FloatField('alpha3', validators=[DataRequired()], widget=NumberInput(min=0, max=1, step='any'), default=1/3)
    sid = HiddenField('sid', validators=[Regexp('^[a-z0-9]+$', message='Invalid session id.')])
    submit = SubmitField('Run!')
    progress_bar = 0
